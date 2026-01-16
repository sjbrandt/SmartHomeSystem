int sPin = 10;
int ledPin = 12;
bool val;

void setup() {
  Serial.begin(115200);
  pinMode(sPin, INPUT);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  int val = digitalRead(sPin);

  if (val == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Motion detected!");
    delay(1000);
  } else {
    digitalWrite(ledPin, LOW);
  }
}
