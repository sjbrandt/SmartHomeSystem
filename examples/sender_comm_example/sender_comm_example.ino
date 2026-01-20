/**
 * @file sender_comm_example.ino
 *
 * @author Sofus Brandt (s214972)
 *
 * @brief This file serves as an example for how to use sender_com.h to send data and fetch commmands.
 *        Do note that it is required to be in range of Victor's hotspot for this to work.
 */

#include "sender_comm.h"
#include <ArduinoJson.h>  // ArduinoJson by Benoit. Needs to be installed through the Library Manager

const int sensorID = 2;  // Every module has an ID
int oldCommandID = -1;

void setup() {
  Serial.begin(115200);

  // Call this first!!! it is also from sender_comm.h
  initWifi();
}

void loop() {
  // ------------ SENDING DATA ------------
  jsonInit(sensorID, "security");       // initialize a json document with sensor ID and sensor type
  jsonAddBool("isLocked", true);        // add a data point of type bool
  jsonAddString("passcode", "1234");    // add a data point of type string
  jsonAddFloat("batteryCharge", 6.7);   // add a data point of type float
  jsonSend();                           // send the JSON document with the data and all!
  
  // The sent JSON will look like this:
  // {
  //   "id": 2,
  //   "ip": "10.198.101.248",
  //   "type": "security",
  //   "data": {
  //     "isLocked": 1,
  //     "passcode": "1234",
  //     "batteryCharge": 6.7
  // }

  // ------------ FETCHING COMMANDS ------------
  // This is how the JSON coming out of fetch_commands will look:
  // {
  //   "targetID": 2,
  //   "cmd": "flash",
  //   "parameters": {
  //     "interval": 1,
  //     "color": "red"
  //   },
  //   "commandID": 179
  // }
  //

  // Call fetch_commands as such to get a string of the JSON data:
  String commandEntry = fetch_commands(sensorID);

  // Make a JsonDocument to store it in. This makes it easier to get data out
  JsonDocument jsonDoc;
  deserializeJson(jsonDoc, commandEntry);  // jsonDoc <- commandEntry

  // May be printed for debugging purposes:
  // ("Pretty" is added to make it more human-readable. One may also call
  // serializeJson() to get a string like the one fetch_commands produces)
  Serial.println(serializeJsonPretty(jsonDoc, Serial));

  // The data is extracted from the jsonDoc as such:
  String cmd = jsonDoc["cmd"];                      // name of the command
  int commandID = jsonDoc["commandID"];             // ID of the command (so you can see whether it is the same old command or a new command to do the same)
  JsonObject parameters = jsonDoc["parameters"];    // "parameters" is a nested JSON object, so you can make it a JsonObject and use it like the JsonDocument
                                                    // Alternatively you may also be able to call jsonDoc["parameters"]["interval"]
  int interval = parameters["interval"].as<int>();  // .as<dataType>() is required for non-string types
  String color = parameters["color"];

  // One may command ID's to check whether we are seeing the same old command or a new one
  bool commandIsNew = (oldCommandID == commandID);
  if (commandIsNew) {
    oldCommandID = commandID;
    // + actually do the command lmao
  }

  // Here I print everything to make sure the variables actually contain what they are supposed to
  Serial.println("");
  Serial.print("cmd = ");
  Serial.println(cmd);
  Serial.print("commandID = ");
  Serial.println(commandID);
  Serial.print("interval = ");
  Serial.println(interval);
  Serial.print("color = ");
  Serial.println(color);


  delay(10000);
}
