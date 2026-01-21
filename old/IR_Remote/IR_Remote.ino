#include <IRremote.hpp>

const uint32_t buttonOne = 0xF30CFF00;
const uint8_t inPin = 8;
bool systemState = HIGH;

void setup(){
    Serial.begin(115200);
    IrReceiver.begin(inPin, ENABLE_LED_FEEDBACK);
}

void loop(){
  if (IrReceiver.decode()) {
    uint32_t receivedCode = IrReceiver.decodedIRData.decodedRawData;
    if (receivedCode == buttonOne && receivedCode != 0) {
      systemState = !systemState;
      if (systemState == 0) {
        Serial.println("Sensor OFF!");
      }
      if (systemState == 1) {
        Serial.println("Sensor ON!");
      }
    }
    IrReceiver.resume();
  }
}
