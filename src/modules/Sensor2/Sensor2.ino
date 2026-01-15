const uint8_t flamePin2 = A1;
const uint8_t ledPin = 10;

void setup() {
  Serial.begin(115200);
  pinMode(flamePin2, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  sensor2();
  delay(1000);

}

void sensor2() {
  int flameDetected = analogRead(flamePin2); // Read the flame sensor

  Serial.println(flameDetected);
  if (flameDetected > 50) {
    digitalWrite(ledPin, HIGH); // Turn on LED if fire is detected
    Serial.println("FIIIIRREEE!!");
  } else {
    digitalWrite(ledPin, LOW);  // Turn off LED if no fire
  }
}