#include "sender_comm.h"

const uint8_t flamePin1 = A0;
const uint8_t ledPin = D2;
const uint8_t sensorID = 4;
uint8_t fireCounter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(flamePin1, INPUT);
  pinMode(ledPin, OUTPUT);
  initWifi();
}

void loop() {
  sensor1();
  delay(300);
}

void blink() {
  digitalWrite(ledPin, HIGH);
  delay(300);
  digitalWrite(ledPin, LOW);
}

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
  } else {
    fireCounter = 0;
  }
}
