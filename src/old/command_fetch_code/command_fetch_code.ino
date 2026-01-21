#include <ArduinoJson.h>

// libraries for ESP32
// #include <WiFi.h>
// #include <HTTPClient.h>

// libraries for ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

WiFiClient client;
HTTPClient http;

const char* ssid = "TheBlackLodge";              // WiFi name
const char* pass = "theowlsarenotwhattheyseem";  // WiFi password

const char* serverAddress = "http://10.198.101.231/api/commands";

String payload = "{}";

int mySensorID = 2;
int lastCommandID;

void handle_command(String cmd, JsonObject parameters) {
  Serial.print("the command is: ");
  Serial.println(cmd);
  Serial.print("the parameters look like this: ");
  Serial.println(serializeJsonPretty(parameters, Serial));
  Serial.println("each value is accessed as such: ");
  Serial.println(parameters["interval"].as<int>());
  Serial.println(parameters["color"].as<String>());
}


void fetch_commands() {
  http.begin(client, serverAddress);

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
    return;
  }
  //Serial.println(serializeJsonPretty(doc, Serial));

  JsonArray list = doc.as<JsonArray>();

  if (list.isNull()) {
    Serial.println("Not a JSON list");
    return;
  }

  for (JsonObject entry : list) {
    int targetID = entry["targetID"];
    if (targetID == mySensorID) {
      if (lastCommandID == entry["commandID"]) {
        return;
      } else {
        lastCommandID = entry["commandID"];
        handle_command(entry["cmd"], entry["parameters"]);
      }
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // start up WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
}

void loop() {
  // put your main code here, to run repeatedly:
  fetch_commands();
  delay(10000);
}
