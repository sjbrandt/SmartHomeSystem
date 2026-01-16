#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

bool testVariable = true;


// this is the webpage
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head><title>Smart Home System Dashboard</title></head>
<body>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Indoor+Temperature&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Indoor+Humidity&type=line"></iframe>
<br>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/3?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=IR+Sensor&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/4?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Proximity+Sensor&type=line"></iframe>
</body></html>
)rawliteral";

const char* ssid = "TheBlackLodge";
const char* pass = "theowlsarenotwhattheyseem";

ESP8266WebServer server(80);

#define MAXSENSORS 10

struct SensorData {
  String ip;
  String type;
  JsonDocument lastData;
  unsigned long lastSeen;
};

SensorData sensors[MAXSENSORS];

struct Command {
  int targetID;
  String cmd;
  JsonDocument parameters;
  bool executed;
};

Command commands[MAXSENSORS];

unsigned long lastThingSpeakUpload = 0;
const unsigned long thingSpeakInterval = 20UL * 1000UL;

// handles sensor to hub communication
void handleSensorPost() {
  if (server.arg("plain").length() == 0) {
    server.send(400, "text/plain", "No body");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));

  if (err) {
    server.send(400, "text/plain", "Bad JSON");
    return;
  }

  int id = doc["id"];
  if (id < 0 || id >= MAXSENSORS) {
    server.send(400, "text/plain", "Invalid ID");
    return;
  }

  sensors[id].ip = doc["ip"].as<String>();
  sensors[id].type = doc["type"].as<String>();
  sensors[id].lastData.clear();
  sensors[id].lastData.set(doc["data"]);
  sensors[id].lastSeen = millis();

  server.send(200, "text/plain", "OK");
  Serial.println(sensors[id].ip);
  Serial.println(sensors[id].type);
  Serial.println(serializeJsonPretty(sensors[id].lastData, Serial));
  Serial.println(sensors[id].lastSeen);
}

// handles dashboard to hub communication
/*
void handleStateGet() {
  JsonDocument doc;
  JsonArray arr = doc["sensors"].to<JsonArray>();

  for (int i = 0; i < MAXSENSORS; i++) {
    if (sensors[i].lastSeen == 0) continue;

    JsonObject o = arr.add<JsonObject>();
    o["id"] = i;
    o["temp"] = sensors[i].temp;
    o["hum"] = sensors[i].hum;
    o["age_ms"] = millis() - sensors[i].lastSeen;
  }

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}
*/

void handleDashboard() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

void handleCommands() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < MAXSENSORS; i++) {
    if (commands[i].executed) continue;

    JsonObject o = arr.add<JsonObject>();
    o["targetID"] = commands[i].targetID;
    o["cmd"] = commands[i].cmd;
    o["parameters"] = commands[i].parameters;  // copy entire params document
  }

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void uploadToThingSpeak() {
  Serial.println("under construction");
}

void setupRoutes() {
  // dashboard page
  server.on("/", HTTP_GET, handleDashboard);

  // API endpoints
  server.on("/api/sensor", HTTP_POST, handleSensorPost);
  //server.on("/api/state", HTTP_GET, handleStateGet); //currently unused
  server.on("/api/commands", HTTP_GET, handleCommands);
  // fallback
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
}

void addCommand(int targetID, const String& cmd, JsonDocument& parameters) {
  for (int i = 0; i < MAXSENSORS; i++) {
    if (!commands[i].executed) {
      commands[i].targetID = targetID;
      commands[i].cmd = cmd;
      commands[i].parameters.clear();
      commands[i].parameters.set(parameters);  // copy everything
      commands[i].executed = false;
      return;
    }
  }
  Serial.println("Command array full!");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid, pass);

  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  setupRoutes();
  server.begin();
  Serial.println("HTTP server started");
}

void evaluateRules() {
  for (int i = 0; i < MAXSENSORS; i++) {
    if ((sensors[i].type == "fire alarm" && sensors[i].lastData["alarm"]) || testVariable) {
      JsonDocument parameters;
      parameters["interval"] = 1;
      parameters["color"] = "red";
      addCommand(2, "flash", parameters);  // sensor ID 2, command, parameters
      testVariable = false;
    }
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  evaluateRules();
  // only periodically uploads to thingspeak such that it doesn't interfere with sensor signals
  if (millis() - lastThingSpeakUpload > thingSpeakInterval) {
    uploadToThingSpeak();
    lastThingSpeakUpload = millis();
  }
}