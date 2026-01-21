#include <IRremote.hpp>

const uint8_t sPin = 10;
const uint8_t ledPin = 12;
const uint8_t rPin = 8;
const uint32_t switchHex = 0xF30CFF00;
int val;
unsigned long timer = 0;
const unsigned long timeout = 5000;

bool ledState = LOW;
bool prevLedState = LOW;
bool systemState = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(sPin, INPUT);
  pinMode(ledPin, OUTPUT);
  IrReceiver.begin(rPin, ENABLE_LED_FEEDBACK);
}

void loop() {

  if (IrReceiver.decode()) {
    uint32_t receivedCode = IrReceiver.decodedIRData.decodedRawData;
    if (receivedCode == switchHex && receivedCode != 0) {
      systemState = !systemState;
      if (systemState == 0) {
        Serial.println("Motion sensor OFF!");
      }
      if (systemState == 1) {
        Serial.println("Motion sensor ON!");
      }
    }
    IrReceiver.resume();
  }

  if (!systemState) {
    digitalWrite(ledPin, LOW);
    return;
  }

  val = digitalRead(sPin);

  if (val == HIGH) {
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