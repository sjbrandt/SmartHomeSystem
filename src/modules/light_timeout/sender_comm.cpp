/**
 * @file sender_comm.cpp
 * 
 * @author Sofus Brandt (s214972)
 * @author Victor Andersen (s244824)
 * 
 * @brief Implementation for `sender_comm.h`, allowing for communication with the Central Hub
 *        through HTTP requests and JSON objects.
 * 
 * @version 0.3
 */

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

JsonDocument currentDoc;  // Json document currently in construction
JsonObject currentData;  // Data part of the Json document currently in construction

/**
 * @brief Logs into WiFi with SSID and password, and logs IP to the serial monitor
 * 
 * @author Sofus Brandt (s214972)
 * @author Victor Andersen (s244824)
 */
void initWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.print("Initialized WiFi. IP: ");
  Serial.println(WiFi.localIP().toString());
}

/**
 * @brief creates a JSON payload in the form of a string
 * 
 * @param sensorID ID of the calling module
 * @param sensorType type/name of sensor/module
 * @param dataName name of given data point
 * @param data the data to be packaged
 * @return String 
 * 
 * @author Sofus Brandt (s214972)
 * @author Victor Andersen (s244824)
 */
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

/**
 * @brief Initializes a JSON document for sending data to teh central hub.
 * 
 * @param sensorID ID of the transmitting module
 * @param sensorType type or name of sensor, to differentiate different sensors in the same module
 * 
 * @author Sofus Brandt (s214972) 
 */
void jsonInit(int sensorID, String sensorType) {
  currentDoc.clear();
  currentDoc["id"] = sensorID;
  currentDoc["ip"] = WiFi.localIP().toString();
  currentDoc["type"] = sensorType;
  currentData = currentDoc["data"].to<JsonObject>();
}

/**
 * @brief Adds a float data point to the JSON document in construction
 * 
 * @param dataName name of the data point (eg. "temperature")
 * @param data data to be transmitted
 * 
 * @author Sofus Brandt (s214972) 
 */
void jsonAddFloat(String dataName, float data) {
  currentData[dataName] = data;
}

/**
 * @brief Adds a bool data point to the JSON document in construction
 * 
 * @param dataName name of the data point (eg. "temperature")
 * @param data data to be transmitted
 * 
 * @author Sofus Brandt (s214972) 
 */
void jsonAddBool(String dataName, bool data) {
  currentData[dataName] = data;
}

/**
 * @brief Adds a string data point to the JSON document in construction
 * 
 * @param dataName name of the data point (eg. "temperature")
 * @param data data to be transmitted
 * 
 * @author Sofus Brandt (s214972) 
 */
void jsonAddString(String dataName, String data) {
  currentData[dataName] = data;
}

/**
 * @brief Serializes and sends the JSON document
 * 
 * @author Sofus Brandt (s214972)
 */
void jsonSend() {
  String payload;
  serializeJson(currentDoc, payload);

  //Serial.println(serializeJsonPretty(currentDoc, Serial));  // DEBUG - print constructed payload

  sendPayload(payload);
}

/**
 * @brief sends a JSON payload to the Central Hub
 * 
 * @param payload JSON payload to be sent
 * 
 * @author Sofus Brandt (s214972)
 * @author Victor Andersen (s244824)
 */
void sendPayload(String payload) {
  http.begin(client, "http://10.198.101.231/api/sensor");  // hub IP
  http.addHeader("Content-Type", "application/json");      // tells the HTTP server what content we're sending
  http.setReuse(false);

  int httpCode = http.POST(payload);  // posts the payload, returns the post code

  if (httpCode > 0) {
    //Serial.print("POST code: ");
    //Serial.println(httpCode);
  } else {
    //Serial.print("POST failed: ");
    //Serial.println(http.errorToString(httpCode));
  }
  http.end();
}

/**
 * @brief DEPRECATED - Use jsonInit(), jsonAdd<Datatype>(), and jsonSend().
 *        Creates a payload of the given data and sends it to the Central Hub.
 * 
 * @param sensorID ID of the calling module
 * @param sensorType type/name of sensor/module
 * @param dataName name of given data point
 * @param data the data to be sent
 * 
 * @author Sofus Brandt (s214972)
 */
void sendSensorData(int sensorID, String sensorType, String dataName, float data) {
  String payload = createPayload(sensorID, sensorType, dataName, data);
  sendPayload(payload);
}

/**
 * @brief Fetches the currently posted commands from the Central Hub and returns the first
 *        command intended for the given sensorID.
 * 
 * @param sensorID the ID for which to fetch a command
 * @return String 
 * 
 * @author Sofus Brandt (s214972)
 * @author Victor Andersen (s244824)
 */
String fetch_commands(int sensorID) {
  String payload = "{}";
  int lastCommandID;

  http.begin(client, "http://10.198.101.231/api/commands");

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    //Serial.print("Error code: ");
    //Serial.println(httpResponseCode);
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

/**
 * @brief Unused function to demonstrate usage of JSON objects
 * 
 * @param cmd command to print/unpack
 * @param parameters parameters to print/unpack
 * 
 * @author Victor Andersen (s244824)
 */
void handle_command(String cmd, JsonObject parameters) {
  Serial.print("the command is: ");
  Serial.println(cmd);
  Serial.print("the parameters look like this: ");
  Serial.println(serializeJsonPretty(parameters, Serial));
  Serial.println("each value is accessed as such: ");
  Serial.println(parameters["interval"].as<int>());
  Serial.println(parameters["color"].as<String>());
}
