const uint8_t flamePin = A0;
const int sensorMax = 1024;
const int sensorMin = 0;
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
  float flameReading = analogRead(flamePin);
  Serial.println(flameReading);
  if (flameReading < 60) {
    Serial.println("FIRE AHHHHHHHHHHHH");
  }
  delay(100);
}
