#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

WiFiClient client;
HTTPClient http;

const char* ssid = "TheBlackLodge"; // WiFi name
const char* pass = "theowlsarenotwhattheyseem"; // WiFi password

void sendSensorData(String payload) {
  http.begin(client, "http://10.198.101.231/api/sensor"); // hub IP
  http.addHeader("Content-Type", "application/json"); // tells the HTTP server what content we're sending

  int httpCode = http.POST(payload); // posts the payload, returns the post code

  if (httpCode > 0) {
    Serial.print("POST code: ");
    Serial.println(httpCode);
  } else {
    Serial.print("POST failed: ");
    Serial.println(http.errorToString(httpCode));
  }
  http.end();
}


void setup() {
  Serial.begin(115200);

  // start up WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
}

void loop() {
  // creates JSON with sensor information
  JsonDocument doc;
  doc["id"] = 1; // MUST be unique from other sensors
  doc["ip"] = WiFi.localIP().toString(); // adds IP of the sensor
  doc["type"] = "dht22"; // what type of sensor is used. can also be called whatever function the sensor does

  // creates JSON with measured sensor data
  JsonObject data = doc["data"].to<JsonObject>();
  data["temperature"] = 21.4;
  data["humidity"] = 48.0;

  // properly serializes the data for transmission
  String payload;
  serializeJson(doc, payload);

  // sends data
  sendSensorData(payload);
  delay(5000);
}
