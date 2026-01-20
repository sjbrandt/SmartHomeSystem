const uint8_t sPin = D4;
const uint8_t ledPin = D2;
int val;
unsigned long timer = 0;
const unsigned long timeout = 5000;

bool ledState = LOW;
bool prevLedState = LOW;
bool systemState = HIGH;
bool isLocked = 0;

void setup() {
  Serial.begin(115200);
  pinMode(sPin, INPUT);
  pinMode(ledPin, OUTPUT);
  // request isLocked?
}

void loop() {
  if (isLocked == 0) {
    Serial.println("Warn HUB of motion");
  } else {
    remoteState();
    motionSensor();
  }
}

void remoteState() {
  motionSensor();
  delay(200);
}

void motionSensor() {
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
