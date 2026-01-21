/**
 * @file Sensor1.ino
 * @author Yun Jie Si Høj (s224179@dtu.dk)
 * @brief Flame detector sensor module code
 * @version 0.1
 * @date 2026-01-21
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "sender_comm.h"

/**
 * @brief Pin definitions and global variables. Each module has its own unique sensorID.
 */
const uint8_t flamePin1 = A0;
const uint8_t ledPin = D2;
const uint8_t sensorID = 4;
uint8_t fireCounter = 0;
/**
 * @brief Setup function to initialize serial communication, pin modes, and WiFi
 * @author Yun Jie Si Høj
 */
void setup() {
  Serial.begin(115200);
  pinMode(flamePin1, INPUT);
  pinMode(ledPin, OUTPUT);
  initWifi();
}

/**
 * @brief Main loop function to read sensor data and handle fire detection
 * @author Yun Jie Si Høj
 */
void loop() {
  sensor1();
  delay(300);
}

/**
 * @brief Function to blink an LED for visual indication
 * @author Yun Jie Si Høj
 */
void blink() {
  digitalWrite(ledPin, HIGH);
  delay(300);
  digitalWrite(ledPin, LOW);
}
/**
 * @brief Function to read flame sensor data and send alert to the HUB if fire is detected.
 * All the json handling is done in sender_comm.h.
 * @author Yun Jie Si Høj
 */
void sensor1() {
  float flameReading = analogRead(flamePin1);
  Serial.println(flameReading);
  if (flameReading < 60) {
    Serial.println("FIRE!");
    blink();
    fireCounter++;
    if (fireCounter >= 5) {
      // ------------ SENDING DATA ------------
      // Initialize a json document with sensor ID and sensor type.
      // This is so you can send different data points at once. New data of the same type should be in a new JSON.
      jsonInit(sensorID, "flameDetector");

      // add a data point of type bool
      jsonAddBool("isFire", true);

      // send the JSON document with the data and all!
      jsonSend();
      Serial.print("Warn HUB!");
    }
  /**
   * @brief Else reset the fireCounter.
   */
  } else {
    fireCounter = 0;
  }
}
