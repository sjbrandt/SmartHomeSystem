#include <IRremote.hpp>

const uint8_t inPin = 11;
const uint8_t ledPin = 8;
const uint32_t firHex = 0xF30CFF00;  // The hex code to match

void setup() {
    Serial.begin(115200);
    IrReceiver.begin(inPin, ENABLE_LED_FEEDBACK);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
}

void blink() {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
}

void loop() {
  if (IrReceiver.decode()) {
    uint32_t receivedCode = IrReceiver.decodedIRData.decodedRawData;
    // Check if the received code is not 0 and matches the target code
    if (receivedCode == firHex && receivedCode != 0) {
      Serial.println("BLINK!");
      blink();
      }
    else if (receivedCode != firHex && receivedCode != 0) {
      Serial.print("Different code received: ");
      Serial.println(receivedCode, HEX);
    }
    IrReceiver.resume();
  }
}
