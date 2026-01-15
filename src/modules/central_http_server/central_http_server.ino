#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

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
  float temp;
  float hum;
  unsigned long lastSeen;
};

SensorData sensors[MAXSENSORS];

unsigned long lastThingSpeakUpload = 0;
const unsigned long thingSpeakInterval = 20UL * 1000UL;

// handles sensor to hub communication
void handleSensorPost() {
  if (!server.hasArg("id") || !server.hasArg("temp") || !server.hasArg("hum")) {
    server.send(400, "text/plain", "Missing fields");
    return;
  }
  int id = server.arg("id").toInt();
  if (id < 0 || id >= MAXSENSORS) {
    server.send(400, "text/plain", "Invalid ID");
    return;
  }

  sensors[id].temp = server.arg("temp").toFloat();
  sensors[id].hum = server.arg("hum").toFloat();
  sensors[id].lastSeen = millis();

  server.send(200, "text/plain", "OK");
}

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


void handleDashboard() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

void uploadToThingSpeak() {
  Serial.println("under construction");
}

void setupRoutes() {
  // dashboard page
  server.on("/", HTTP_GET, handleDashboard);

  // API endpoints
  server.on("/api/sensor", HTTP_POST, handleSensorPost);
  server.on("/api/state", HTTP_GET, handleStateGet);

  // fallback
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
}

void setup() {
  // put your setup code here, to run once:
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

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient();
  // only periodically uploads to thingspeak such that it doesn't interfere with sensor signals
  if (millis() - lastThingSpeakUpload > thingSpeakInterval) {
    uploadToThingSpeak();
    lastThingSpeakUpload = millis();
  }
}
