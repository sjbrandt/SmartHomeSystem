#include "comm.h"
#include <ArduinoJson.h>  // ArduinoJson by Benoit

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
#endif

WiFiClient client;
HTTPClient http;

const char* ssid = "TheBlackLodge"; // WiFi name
const char* pass = "theowlsarenotwhattheyseem"; // WiFi password

String createPayload(int sensorID, String sensorType, String dataName, bool data) {
  JsonDocument doc;
  doc["id"] = sensorID; // MUST be unique from other sensors
  doc["ip"] = WiFi.localIP().toString(); // adds IP of the sensor
  doc["type"] = sensorType; // what type of sensor is used. can also be called whatever function the sensor does

  // creates JSON with measured sensor data. you can add however many data fields as necessary
  JsonObject dataJson = doc["data"].to<JsonObject>();
  dataJson[dataName] = data;

  // properly serializes the data for transmission
  String payload;
  serializeJson(doc, payload);

  return payload;
}

void sendPayload(String payload) {
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

void sendSensorData(int sensorID, String sensorType, String dataName, bool data) {
  String payload = createPayload(sensorID, sensorType, dataName, data);
  sendPayload(payload);
}

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
}
