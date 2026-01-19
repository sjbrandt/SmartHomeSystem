#include <IRremote.hpp>

const uint8_t flamePin1 = A0;
const uint8_t aPin = 10;
const uint8_t remPin = 8;
const uint32_t buttonTwo = 0xE718FF00;
bool systemState = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(flamePin1, INPUT);
  pinMode(aPin, OUTPUT);
  IrReceiver.begin(remPin, ENABLE_LED_FEEDBACK);
}

void loop() {
  if (IrReceiver.decode()) {
    uint32_t receivedCode = IrReceiver.decodedIRData.decodedRawData;
    if (receivedCode == buttonTwo && receivedCode != 0) {
      systemState = !systemState;
      if (systemState == 0) {
        Serial.println("Flame sensor OFF!");
      }
      if (systemState == 1) {
        Serial.println("Flame sensor ON!");
      }
    }
    IrReceiver.resume();
  }

  if (!systemState) {
    return;
  }

  sensor1();
  delay(1000);
}

void blink() {
  digitalWrite(aPin, HIGH);
  delay(500);
  digitalWrite(aPin, LOW);
}
void sensor1() {
  float flameReading = analogRead(flamePin1);
  Serial.println(flameReading);
  if (flameReading < 60) {
    Serial.println("Noot Noot!");
    blink();
  }
}

