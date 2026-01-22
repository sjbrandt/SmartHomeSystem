/**
 * @file project_temp_sensor.ino
 * @brief Sketch for the temperature module
 *
 * This sketch is the code to be run on the ESP8266 controlling the temperature module.
 *
 * Sensors:
 * - LM35-DZ Analog temperature sensor
 *
 * Actuators:
 * - Red LED
 * - DC Motor/Fan
 *
 *
 * @author Deniz Coskun (s225730)
 * @date Januaray 2026
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include "sender_comm.h"

// USER CONFIG
const int LED_PIN = D3;
const int FAN_PIN = D7;

const float T_DESIRED = 27.0;

// temp ranges used for fan pwm logic
const float T_MIN = T_DESIRED + 3.0;   // fan starts here
const float T_MAX = T_DESIRED + 10.0;   // full speed here

// temp ranges used for LED pwm
const float LED_COLD = T_DESIRED - 10.0; // full bright at/below this
const float LED_OFF  = T_DESIRED - 3.0; // off at/above this

// global variables used
float simOffsetC = 0.0;
float prevTempSample = 0.0;

// used for communication
const int sensorID = 2;  // temp module ID is 2, every module has its own ID
//int oldCommandID = -1;

// function for reading sensor
/**
 * @brief Reads temperature from A0 pin on ESP8266
 * 
 * @return float temp
 * 
 * @author Deniz Coskun (s225730)
 */
float readTemp() {
  int raw = analogRead(A0);
  float voltage = raw * (3.3 / 1023.0);
  float tempC = voltage * 100.0;
  return tempC;
}

// function for adding temperature through serial monitor, used for testing purposes
/**
 * @brief Adds offset to temperature through serial monitor, mainly used for testing and showing LED/FAN state in different temperatures
 * 
 * @author Deniz Coskun (s225730)
 */
void readSerialOffset() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '+') simOffsetC += 1.0;
    if (c == '-') simOffsetC -= 1.0;
    if (c == '0') simOffsetC = 0.0;
  }
}

// fan pwm
/**
 * @brief Calculates fan PWM based on temperature.
 * 
 * @param tempSample Current temperature
 * @return pwm
 * 
 * @author Deniz Coskun (s225730)
 */
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
/**
 * @brief Calculates LED PWM based on temperature.
 * 
 * @param tempSample Current temperature
 * @return pwm
 * 
 * @author Deniz Coskun (s225730)
 */
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

// -------------------- Setup / Loop -------------------
/**
 * @brief Runs setup functions when the program is first loaded onto the MCU.
 * 
 * @author Deniz Coskun (s225730)
 */
void setup() {
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  delay(50);
  Serial.println();
  Serial.println("Temperature module started.");

  initWifi(); // used for initializing wifi communication
}

/**
 * @brief Runs the main functionality in an infinite loop.
 * 
 * @author Deniz Coskun (s225730)
 */
void loop() {

  //Read temperature
  float tempRead = readTemp();
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
  

  // // communicate
  jsonInit(sensorID, "Temperature");
  jsonAddFloat("tempRead", tempRead);
  jsonAddFloat("fanDuty", fanDuty);
  jsonAddFloat("ledDuty", ledDuty);
  jsonSend(); // send data


  // save prev sample
  prevTempSample = tempSample;

  delay(5000);
}