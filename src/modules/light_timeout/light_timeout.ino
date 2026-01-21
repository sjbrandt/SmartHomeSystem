/**
 * @file Light_timeout.ino
 * @author Yun Jie Si Høj (s224179@dtu.dk) and Frederikke Biehe (s223981@dtu.dk)
 * @brief Motion sensor with light timeout and remote lock functionality.
 * The light turns on when motion is detected and turns off after a timeout period.
 * The system can be remotely locked to disable the light functionality and send alerts on motion detection.
 * @version 0.1
 * @date 2026-01-21
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "sender_comm.h"
#include <ArduinoJson.h>  // ArduinoJson by Benoit. Needs to be installed through the Library Manager

/**
 * @brief Variables and global constants
 * Every module must have a unique sensorID
 */
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

/**
 * @brief Setup function to initialize WiFi, serial communication, and pin modes.
 * @author Yun Jie Si Høj
 */
void setup() {
  initWifi();
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

/**
 * @brief Main loop function to read sensor values, fetch isLocked, and control the light based on motion detection and lock state.
 * @author Yun Jie Si Høj
 */
void loop() {
  sensorVal = digitalRead(sensorPin);

  /**
   * @brief Call fetch_commands as such to get a string of the JSON data
   */
  
  String commandEntry = fetch_commands(sensorID);
  /**
   * @brief Make a JsonDocument to store it in. This makes it easier to get data out.
   * "parameters" is a nested JSON object, so you can make it a JsonObject and use it like the JsonDocument.
   * .as<dataType>() is required for non-string types
   */
  JsonDocument jsonDoc;
  deserializeJson(jsonDoc, commandEntry);         // jsonDoc <- commandEntry
  JsonObject parameters = jsonDoc["parameters"];  
  isLocked = parameters["state"].as<bool>();

  //Serial.print("isLocked: ");
  //Serial.println(isLocked);
  /**
   * @brief If the system is locked, turn off the LED and send an alert on motion detection. Else, run the motion sensor function.
   */
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

/**
 * @brief Function to handle motion sensor logic, turning the LED on when motion is detected and off after a timeout.
 * Timeout can be set in the global constant.
 * @author Yun Jie Si Høj
 */
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
