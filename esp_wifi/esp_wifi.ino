#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
ADC_MODE(ADC_VCC);

#include "DHT.h"         // https://github.com/adafruit/DHT-sensor-library

#define DHTPIN 2
#define DHTTYPE DHT22
#define VCC_ADJ 1.096

const char* ssid = "ssid";
const char* pass = "pass";
const char* host = "192.168.1.203";
const int   port = 3000;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
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

  dht.begin();
}

void loop() {

  // Read sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float v = ESP.getVcc() * VCC_ADJ;

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("failed to read from DHT sensor!");
  } else {

    // Compute heat index in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
  
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C\t");
    Serial.print("Heat index: ");
    Serial.print(hic);
    Serial.print(" *C\t");
    Serial.print(v);
    Serial.println("V");
  
    send_data(t, h, v);
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

  String data = "bathroom t=";
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

