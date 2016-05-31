#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson
#include "DHT.h"         // https://github.com/adafruit/DHT-sensor-library

#define DHTPIN 2
#define DHTTYPE DHT22

const char* ssid     = "name";
const char* password = "pass";

// Local IP adress where is the server
const char* host = "192.168.1.203";

StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

DHT dht(DHTPIN, DHTTYPE);

class ByteCounter : public Print {
  public:
    ByteCounter(): len(0) {}

    virtual size_t write(uint8_t c) {
      len++;
    }

    int length() const {
      return len;
    }

  private:
    int len;
};

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  root["device"] = "wifi";
}

void loop() {
  delay(5000);

  // Read sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("failed to read from DHT sensor!");
    return;
  }

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
  Serial.println(" *C");

  // Send data
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 3000;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // This will send the request to the server
  /*client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");*/
  root["temperature"] = t;
  root["humidity"] = h;

  ByteCounter counter;
  root.printTo(counter);
  int contentLength = counter.length();

  client.println("POST /api/measurements HTTP/1.1");
  client.print("Host: "); client.println(host);
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(contentLength);
  client.println();
  root.printTo(client);
  client.println();
  delay(500);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

  // Wait 53 secs, total 60 secs
  // (cca 7 secs for sensor reading and request sending)
  delay(53000);
}

