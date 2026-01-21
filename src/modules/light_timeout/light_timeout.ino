


#include "sender_comm.h"
#include <ArduinoJson.h>  // ArduinoJson by Benoit. Needs to be installed through the Library Manager


const int sensorID = 3;  // Every module has an ID
int oldCommandID = -1;

const uint8_t sensorPin = D4;
const uint8_t ledPin = D8;
int sensorVal;
unsigned long timer = 0;
const unsigned long timeout = 5000;

bool ledState = LOW;
bool prevLedState = LOW;
bool systemState = HIGH;
bool isLocked;

void setup() {
  initWifi();
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  sensorVal = digitalRead(sensorPin);

  // Call fetch_commands as such to get a string of the JSON data:
  String commandEntry = fetch_commands(sensorID);
  // Make a JsonDocument to store it in. This makes it easier to get data out
  JsonDocument jsonDoc;
  deserializeJson(jsonDoc, commandEntry);         // jsonDoc <- commandEntry
  JsonObject parameters = jsonDoc["parameters"];  // "parameters" is a nested JSON object, so you can make it a JsonObject and use it like the JsonDocument

  isLocked = parameters["state"].as<bool>();  // .as<dataType>() is required for non-string types

  //Serial.print("isLocked: ");
  //Serial.println(isLocked);

  if (isLocked == true) {
    digitalWrite(ledPin, LOW);
    if (sensorVal == HIGH) {
      Serial.println("Warn HUB of motion");
      

      // Initialize a json document with sensor ID and sensor type.
      // This is so you can send different data points at once. New data of the same type should be in a new JSON.
      jsonInit(sensorID, "motionsensor");

      // add a data point of type bool
      jsonAddBool("isIntruder", true);

      // send the JSON document with the data and all!
      jsonSend();
    }
    delay(5000);
  } else {
    motionSensor();
  }
}


void motionSensor() {
  if (sensorVal == HIGH) {
    digitalWrite(ledPin, HIGH);
    ledState = HIGH;
    timer = millis();
  } 

   if (millis() - timer >= timeout) {
     digitalWrite(ledPin, LOW);
     ledState = LOW;
   }
  if (ledState != prevLedState) {
     if (ledState == HIGH) {
       Serial.println("Motion!");
     } else {
       Serial.println("Timeout.");
     }
     prevLedState = ledState;
   }
}
