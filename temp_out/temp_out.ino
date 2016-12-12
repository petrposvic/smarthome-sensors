#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <OneWire.h>
ADC_MODE(ADC_VCC);

#define PIN 2
#define VCC_ADJ 1.096

const char* ssid = "ssid";
const char* pass = "pass";
const char* host = "192.168.1.203";
const int   port = 3000;

// 4.7K resistor is necessary
OneWire ds(PIN);

void setup(void) {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop(void) {
  float t = get_temp();
  float v = ESP.getVcc() * VCC_ADJ;

  if (t <= -999) {
    Serial.println("failed to read from sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(t);

    send_data(t, 0.0, v);
  }

  // Sleep for 295 secs. Power save deep sleep for ESP where GPIO16 and
  // RST are soldered
  delay(295000);
  // ESP.deepSleep(295 * 1000000);
}

void send_data(float t, float h, float v) {
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }

  String data = "terrace t=";
  data += t;
  data += ",h=";
  data += h;
  data += ",v=";
  data += v;

  client.println("POST /write?db=smarthome HTTP/1.1");
  client.print("Host: "); client.println(host);
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/json");
  client.println("Authorization: Basic cGhvZW5peDpoZXNsbw==");
  client.print("Content-Length: ");
  client.println(data.length());
  client.println();
  client.println(data);
  client.println();
  delay(500);

  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

float get_temp() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];

  int counter = 0;
  while (!ds.search(addr)) {
    ds.reset_search();
    delay(250);

    if (++counter > 8) {
      Serial.println("No more addresses!");
      return -1000;
    }
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return -2000;
  }

  switch (addr[0]) {
    // DS18S20 or old DS1820
    case 0x10:
      type_s = 1;
      break;

    // DS18B20
    case 0x28:
      type_s = 0;
      break;

    // DS1822
    case 0x22:
      type_s = 0;
      break;

    // Not a DS18x20 family device
    default:
      return -3000;
    }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);  // Start conversion, with parasite power on at the end

  delay(1000);        // Maybe 750ms is enough, maybe not
  // We might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);     // Read Scratchpad

  for (i = 0; i < 9; i++) {
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }

  // Celsius
  return (float) raw / 16.0;

  // Fahrenheit
  // return ((float) raw / 16.0) * 1.8 + 32.0;
}
