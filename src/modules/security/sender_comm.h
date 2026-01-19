#ifndef SENSORSENDER_H
#define SENSORSENDER_H

#include <ArduinoJson.h>

#if defined(ESP8266)
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
#elif defined(ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
#endif

void initWifi();
void sendPayload(String payload);
String createPayload(int sensorID, String sensorType, String dataName, float data);
void sendSensorData(int sensorID, String sensorType, String dataName, float data);

String fetch_commands(int sensorID);
void handle_command(String cmd, JsonObject parameters);

#endif