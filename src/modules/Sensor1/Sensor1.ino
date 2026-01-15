const uint8_t flamePin1 = A0;
const uint8_t aPin = 8;

void setup() {
  Serial.begin(115200);
  pinMode(flamePin1, INPUT);
  pinMode(aPin, OUTPUT);
}

void loop() {
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

