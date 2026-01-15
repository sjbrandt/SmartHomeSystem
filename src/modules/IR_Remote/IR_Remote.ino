#include <IRremote.hpp>

const uint8_t inPin = 11;
void setup()
{
    Serial.begin(115200);
    IrReceiver.begin(inPin, ENABLE_LED_FEEDBACK);
}

void loop()
{
    if (IrReceiver.decode())
    {
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        IrReceiver.resume();
    }
}
