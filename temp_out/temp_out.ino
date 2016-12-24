#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <OneWire.h>
#include <DallasTemperature.h>
ADC_MODE(ADC_VCC);

#define PIN 2
#define VCC_ADJ 1.096

const char* ssid = "ssid";
const char* pass = "pass";
const char* host = "192.168.1.203";
const int   port = 3000;

// 4.7K resistor is necessary
OneWire ds(PIN);
DallasTemperature sensors(&ds);

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

  sensors.begin();
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
  int count = 1;
  while (true) {
    sensors.requestTemperatures();
    float temp = sensors.getTempCByIndex(0);

    // Errors:
    // -127   ... device disconnected C
    // -196.6 ... device disconnected F
    // -7040  ... device disconnected raw
    if (temp < -125) {
      if (++count < 10) {
        Serial.print("error ");
        Serial.print(temp);
        Serial.print(", try #");
        Serial.println(count);
        delay(6000);
        continue;
      } else {
        Serial.print("return error code ");
        Serial.println(temp);
      }
    }

    return temp;
  }
}
