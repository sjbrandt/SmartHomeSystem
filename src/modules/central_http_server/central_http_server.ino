/**
 * @file central_http_server.ino
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk)
 * @brief Sketch for the central ESP8266 HTTP server
 * @date 2026
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ThingSpeak.h>

// HTML code for the dashboard
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html><head><title>Smart Home System Dashboard</title></head>
<body>
<center>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Indoor+Temperature&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/widgets/1207936"></iframe>
<br>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Fan+Power+%28PWM%29&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/widgets/1207941"></iframe>
<br>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/3?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Heat+power+%28PWM%29&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/widgets/1207942"></iframe>
<br>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/4?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Door+locked&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/widgets/1207943"></iframe>
<br>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/5?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Fire+alarm&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/widgets/1207944"></iframe>
<br>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/charts/6?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&title=Intruder%3F&type=line"></iframe>
<iframe width="450" height="260" style="border: 1px solid #cccccc;" src="https://thingspeak.com/channels/3229693/widgets/1207946"></iframe>
</center>
</body></html>
)rawliteral";

// -------------------- Configuartion --------------------
const char* ssid = "TheBlackLodge";
const char* pass = "theowlsarenotwhattheyseem";

unsigned long channelID = 3229693;
const char* APIKey = "Q3X1R9DC29KFQM22";
const char* thingSpeakServer = "api.thingspeak.com";

const unsigned long thingSpeakInterval = 20UL * 1000UL;  // Desired seconds * 1000 milliseconds

const unsigned long evaluationInterval = 250;

const unsigned long inactivityInterval = 60000;

bool testVariable = true;

const uint8_t greenLED = D3;
const uint8_t redLED = D4;

WiFiClient client;

// -------------------- Global variabes ------------------------
int currentCommandSlot = 0;
unsigned long lastThingSpeakUpload = 0;
unsigned long lastEvaluation = 0;
unsigned long lastInactivityCheck = 0;
#define MAXSENSORS 10

// -------------------- Structs ------------------------
struct SensorData {
  String ip;
  String type;
  JsonDocument lastData;
  unsigned long lastSeen;
};

SensorData sensors[MAXSENSORS];  // Allocates memory for MAXSENSORS number of sensors

struct Command {
  int targetID;
  String cmd;
  JsonDocument parameters;
  int commandID;
  int lastID;
};

Command commands[MAXSENSORS];  // Allocates memory for MAXSENSORS number of commands

// Opens a webserver on port 80
ESP8266WebServer server(80);

/**
 * @brief Handles sensor to hub communication.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk)
 * 
 */
void handleSensorPost() {
  // Checks whether the request is empty
  if (server.arg("plain").length() == 0) {
    server.send(400, "text/plain", "No body");
    return;
  }

  // Deserializes the request into the JSON object 'doc'. If it returns an error, throw an error message
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server.arg("plain"));
  if (err) {
    server.send(400, "text/plain", "Bad JSON");
    return;
  }

  // Reads and sets the ID of the sensor and checks validity
  int id = doc["id"];
  if (id < 0 || id >= MAXSENSORS) {
    server.send(400, "text/plain", "Invalid ID");
    return;
  }

  // Write the incoming data into a struct with an ID
  sensors[id].ip = doc["ip"].as<String>();
  sensors[id].type = doc["type"].as<String>();
  sensors[id].lastData.clear();
  sensors[id].lastData.set(doc["data"]);
  sensors[id].lastSeen = millis();

  // Lets the sensor know we have received and processes the POST
  server.send(200, "text/plain", "OK");

  Serial.println("Received data:");
  Serial.println(sensors[id].ip);
  Serial.println(sensors[id].type);
  Serial.println(serializeJsonPretty(sensors[id].lastData, Serial));
  Serial.println(sensors[id].lastSeen);
  Serial.println("-------------------");
}

// handles dashboard to hub communication
/**
 * @brief Handles communication from the dashboard to the hub.
 *        Currently unused, however the intent was to be able to interact
 *        with the hub from the website.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk)
 */
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

/**
 * @brief Sends the HTML file through HTTP to whomever requested the file.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk)
 */
void handleDashboard() {
  server.send(200, "text/html", DASHBOARD_HTML);
}

/**
 * @brief Handles sending  the commands to the API for sensors to read and execute.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk)
 * 
 */
void handleCommands() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();

  for (int i = 0; i < MAXSENSORS; i++) {
    JsonObject o = arr.add<JsonObject>();
    o["targetID"] = commands[i].targetID;
    o["cmd"] = commands[i].cmd;
    o["parameters"] = commands[i].parameters;
    o["commandID"] = commands[i].commandID;
  }

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

/**
 * @brief Handles sending select sensor data to ThingSpeak to display graphically.
 * 
 *  @author Victor Kappelhøj Andersen (s244824@dtu.dk) 
 * 
 */
void uploadToThingSpeak() {
  int status = ThingSpeak.writeFields(channelID, APIKey);
  if (status == 200) {
    Serial.println("Data successfully uploaded to ThingSpeak!");
  } else if (status == -210) {
    Serial.println("No data to upload");
  } else {
    Serial.print("Error uploading to ThingSpeak. HTTP Error code: ");
    Serial.println(status);
  }
  Serial.println("-------------------");
  client.stop();
}

/**
 * @brief Sets up all HTTP endpoints for the webserver
 *        / GET             : Serves the dashboard webpage
 *        /api/sensor POST  : Receives sensor data
 *        /api/commands GET : Returns current command list 
 *        And includes sending a 404 in case of no endpoint.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk) 
 */
void setupRoutes() {
  server.on("/", HTTP_GET, handleDashboard);  // Dashboard

  // API endpoints
  server.on("/api/sensor", HTTP_POST, handleSensorPost);  // Sensor data
  //server.on("/api/state", HTTP_GET, handleStateGet); //currently unused
  server.on("/api/commands", HTTP_GET, handleCommands);  // Serves commands
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");  // 404 Fallback
  });
}

/**
 * @brief Adds a command to the list of commands at /api/commands.
 * 
 * @param targetID Defines which sensor to issue a command to.
 * @param cmd Defines the command as a string for the sensor to interpret.
 * @param parameters Defines a JSON object of parameters for the command to execute on.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk) 
 * 
 */
void addCommand(int targetID, const String& cmd, JsonDocument& parameters) {
  /*
  int i = currentCommandSlot;
  // Checks for duplicates
  for (int j = 0; j < MAXSENSORS; j++) {
    if (targetID == commands[j].targetID && cmd == commands[j].cmd && parameters.as<JsonVariant>() == commands[j].parameters.as<JsonVariant>()) {
      return;
    }
  }*/
  for (int i = 0; i < MAXSENSORS; i++) {
    if (commands[i].targetID == targetID) {
      // Assigns the values to the command
      commands[i].targetID = targetID;
      commands[i].cmd = cmd;
      commands[i].parameters.clear();
      commands[i].parameters.set(parameters);
      commands[i].lastID = commands[i].commandID;

      // Gives the command a unique ID, such that sensor can keep track if the command has been executed
      int newID = random(1, 999);
      if (newID == commands[i].lastID) { newID++; };
      commands[i].commandID = newID;
      return;
    }
  }
  for (int i = 0; i < MAXSENSORS; i++) {
    if (commands[i].targetID == 0) {
      // Assigns the values to the command
      commands[i].targetID = targetID;
      commands[i].cmd = cmd;
      commands[i].parameters.clear();
      commands[i].parameters.set(parameters);
      commands[i].lastID = commands[i].commandID;

      // Gives the command a unique ID, such that sensor can keep track if the command has been executed
      int newID = random(1, 999);
      if (newID == commands[i].lastID) { newID++; };
      commands[i].commandID = newID;
      return;
    }
  }
  /*
  // Increments and rolls over the counter to write the next command
  currentCommandSlot++;
  if (currentCommandSlot >= MAXSENSORS) {
    currentCommandSlot = 0;
  }*/
}

/**
 * @brief The Arduino setup function, that runs at boot.
 *        - Sets baud rate
 *        - Sets up LEDs
 *        - Initializes the WiFi
 *        - Attempt to connect to the WiFi network
 *        - Sets up the routes and webserver
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk) 
 */
void setup() {
  Serial.begin(115200);

  // Status LEDs
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, LOW);

  // Initializes the WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  // Attempts to connect to the WiFi (Red LED turns on)
  Serial.println("Connecting");
  digitalWrite(redLED, HIGH);
  int time_to_connect = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
    // If connecting takes more than 15 seconds, try again by restarting everything
    if (millis() - time_to_connect > 15000) {
      Serial.println("Couldn't connect, restarting ...");
      ESP.reset();
    }
  }

  // When successful, IP is printed, routes are set up and the webserver is started
  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  setupRoutes();
  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(redLED, LOW);
  digitalWrite(greenLED, HIGH);

  ThingSpeak.begin(client);
  Serial.println("ThingSpeak initialized.");

  Serial.println("-------------------");
}

/**
 * @brief Here user defined actions can be set up when certain sensor conditions are met.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk) 
 * 
 */
void evaluateRules() {
  // Iterates through all sensors
  for (int i = 0; i < MAXSENSORS; i++) {
    if ((sensors[i].type == "fire alarm" && sensors[i].lastData["alarm"]) || testVariable) {
      JsonDocument parameters;
      parameters["interval"] = 1;
      parameters["color"] = "red";
      addCommand(2, "flash", parameters);  // Sensor ID, Command, Parameters
    }

    if (i == 1 && sensors[i].type == "security") {
      if (millis() - sensors[i].lastSeen < 100) {
        ThingSpeak.setField(4, sensors[i].lastData["isLocked"].as<bool>());
      }

      JsonDocument parameters;
      parameters["state"] = sensors[i].lastData["isLocked"].as<bool>();
      addCommand(3, "checkState", parameters);
    }

    if (i == 2 && sensors[i].type == "Temperature") {
      if (millis() - sensors[i].lastSeen < 100) {
        float reading = sensors[i].lastData["tempRead"].as<float>();
        float rounded_value;
        rounded_value = roundf(reading * 100) / 100;
        ThingSpeak.setField(1, rounded_value);
        ThingSpeak.setField(2, sensors[i].lastData["fanDuty"].as<int>());
        ThingSpeak.setField(3, sensors[i].lastData["ledDuty"].as<int>());
      }
    }

    if (i == 3 && sensors[i].type == "motionsensor") {
      if (millis() - sensors[i].lastSeen < 100) {
        ThingSpeak.setField(6, sensors[i].lastData["isIntruder"].as<bool>());
      }
      if ((millis() - sensors[i].lastSeen > inactivityInterval) && (millis() - lastInactivityCheck > inactivityInterval))  {
        ThingSpeak.setField(6, false);
      }
    }
    
    if (i == 4 && sensors[i].type == "flameDetector") {
      if (millis() - sensors[i].lastSeen < 100) {
        ThingSpeak.setField(5, sensors[i].lastData["isFire"].as<bool>());
      }
      if ((millis() - sensors[i].lastSeen > inactivityInterval) && (millis() - lastInactivityCheck > inactivityInterval)) {
        ThingSpeak.setField(5, false);
        lastInactivityCheck = millis();
      }
    }
  }
  testVariable = false;  // Used for internal testing purposes.
}

/**
 * @brief The Arduino loop listens for incoming HTTP requests and evaluates the rules/user defined actions.
 *        Once ThingSpeak is set up, it uploads its data occasionally.
 * 
 * @author Victor Kappelhøj Andersen (s244824@dtu.dk) 
 */
void loop() {
  server.handleClient();

  if (millis() - lastEvaluation > evaluationInterval) {
    evaluateRules();
    lastEvaluation = millis();
  }

  // Only periodically uploads to ThingSpeak, such that it doesn't interfere with incoming data
  if (millis() - lastThingSpeakUpload > thingSpeakInterval) {
    uploadToThingSpeak();
    lastThingSpeakUpload = millis();
  }
}