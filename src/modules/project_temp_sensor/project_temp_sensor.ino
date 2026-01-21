#include <Arduino.h>

// communication
#include "sender_comm.h"
#include <ArduinoJson.h>  // ArduinoJson by Benoit. Needs to be installed through the Library Manager

// pins
const int LED_PIN = D3;
const int FAN_PIN = D7;

const float T_DESIRED = 25.0;

// temp ranges used for fan pwm logic
// Fan range
const float T_MIN = T_DESIRED + 3.0;   // fan starts here
const float T_MAX = T_DESIRED + 10.0;   // full speed here

// temp ranges used for LED pwm
const float LED_COLD = T_DESIRED - 10.0; // full bright at/below this
const float LED_OFF  = T_DESIRED - 1.0; // off at/above this

// global variables used
float simOffsetC = 0.0;
float prevTempSample = 0.0;

// used for communication
const int sensorID = 2;  // temp module ID is 2, every module has its own ID
//int oldCommandID = -1;

// function for reading sensor
float readTemp() {
  int raw = analogRead(A0);
  float voltage = raw * (3.3 / 1023.0);
  float tempC = voltage * 100.0;
  return tempC;
}

// function for adding temperature through serial monitor, used for testing purposes
void readSerialOffset() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '+') simOffsetC += 1.0;
    if (c == '-') simOffsetC -= 1.0;
    if (c == '0') simOffsetC = 0.0;
  }
}

// fan pwm
int computeFanPWM(float tempSample) {
  int pwm = 0;

  if (tempSample < T_MIN && prevTempSample < T_MIN) { // dont spin below this threshold
    pwm = 0;
  } else { // start spinning above 26 degrees, and spin faster for hotter temperature
    pwm = (int)((tempSample - T_MIN) * 255.0 / (T_MAX - T_MIN));
    pwm = constrain(pwm, 0, 255);

    const int PWM_MIN_SPIN = 80;   // minimum ~30% duty cycle
    if (pwm > 0 && pwm < PWM_MIN_SPIN) pwm = PWM_MIN_SPIN;
  }
  return pwm;
}

// led PWM
int computeLedPWM(float tempSample) {
  int pwm = 0;

  if (tempSample <= LED_COLD) {
    pwm = 255;                     // full bright/heating
  } 
  else if (tempSample < LED_OFF) {
    pwm = (int)((LED_OFF - tempSample) * 255.0 / (LED_OFF - LED_COLD));
    pwm = constrain(pwm, 0, 255);
  } 
  else {
    pwm = 0; // off above desired temp
  }
  return pwm;
}

void setup() {
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("Temperature module started.");

  initWifi(); // used for initializing wifi communication
}

void loop() {

  //Read temperature
  float tempRead = readTemp() - 5.0;
  readSerialOffset();
  tempRead += simOffsetC;

  float tempSample = tempRead;

  // pwm values
  int fanPwm = computeFanPWM(tempSample);
  int ledPwm = computeLedPWM(tempSample);

  // write outputs
  analogWrite(FAN_PIN, fanPwm);
  analogWrite(LED_PIN, ledPwm);

  // print status
  int fanDuty = map(fanPwm, 0, 255, 0, 100);
  int ledDuty = map(ledPwm, 0, 255, 0, 100);

  Serial.print("Temperature: ");
  Serial.print(tempRead, 2);
  Serial.print(" °C");
  Serial.print(" | Simoffset: ");
  Serial.print(simOffsetC, 1);
  Serial.print(" | Fan duty: ");
  Serial.print(fanDuty);
  Serial.print("% | LED duty: ");
  Serial.print(ledDuty);
  Serial.println("%");

  // communicate
  jsonInit(sensorID, "Temperature");
  jsonAddFloat("tempRead", tempRead);
  jsonAddFloat("fanDuty", fanDuty);
  jsonAddFloat("ledDuty", ledDuty);
  jsonSend(); // send data


  // save prev sample
  prevTempSample = tempSample;

  delay(500);
}