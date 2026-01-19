#include "sender_comm.h"
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

const char* ssid = "TheBlackLodge";              // WiFi name
const char* pass = "theowlsarenotwhattheyseem";  // WiFi password

void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Initialized WiFi. IP: ");
  Serial.println(WiFi.localIP().toString());
}

// creates a JSON payload, in the form of a string, containing data (currently only boolean)
String createPayload(int sensorID, String sensorType, String dataName, float data) {
  JsonDocument doc;
  doc["id"] = sensorID;                   // MUST be unique from other sensors
  doc["ip"] = WiFi.localIP().toString();  // adds IP of the sensor
  doc["type"] = sensorType;               // what type of sensor is used. can also be called whatever function the sensor does

  // creates JSON with measured sensor data. you can add however many data fields as necessary
  JsonObject dataJson = doc["data"].to<JsonObject>();
  dataJson[dataName] = data;

  // properly serializes the data for transmission
  String payload;
  serializeJson(doc, payload);

  Serial.println(serializeJsonPretty(doc, Serial));  // DEBUG - print constructed payload

  return payload;
}

void sendPayload(String payload) {
  http.begin(client, "http://10.198.101.231/api/sensor");  // hub IP
  http.addHeader("Content-Type", "application/json");      // tells the HTTP server what content we're sending

  int httpCode = http.POST(payload);  // posts the payload, returns the post code

  if (httpCode > 0) {
    Serial.print("POST code: ");
    Serial.println(httpCode);
  } else {
    Serial.print("POST failed: ");
    Serial.println(http.errorToString(httpCode));
  }
  http.end();
}

void sendSensorData(int sensorID, String sensorType, String dataName, float data) {
  String payload = createPayload(sensorID, sensorType, dataName, data);
  sendPayload(payload);
}

String fetch_commands(int sensorID) {
  String payload = "{}";
  int lastCommandID;

  http.begin(client, "http://10.198.101.231/api/commands");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload);
  if (err) {
    Serial.println(err.c_str());
    return "";
  }
  //Serial.println(serializeJsonPretty(doc, Serial));

  JsonArray list = doc.as<JsonArray>();

  if (list.isNull()) {
    Serial.println("Not a JSON list");
    return "";
  }

  for (JsonObject entry : list) {
    int targetID = entry["targetID"];
    if (targetID == sensorID) {
      String entryStr;
      serializeJson(entry, entryStr);
      return entryStr;
    }
  }
  return "";
}

void handle_command(String cmd, JsonObject parameters) {
  Serial.print("the command is: ");
  Serial.println(cmd);
  Serial.print("the parameters look like this: ");
  Serial.println(serializeJsonPretty(parameters, Serial));
  Serial.println("each value is accessed as such: ");
  Serial.println(parameters["interval"].as<int>());
  Serial.println(parameters["color"].as<String>());
}
