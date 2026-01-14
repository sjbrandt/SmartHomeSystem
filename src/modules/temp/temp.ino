const int PIN_LED_HEAT = D0;
const int PIN_LED_FAN = D1;
const int PIN_TEMP = A0;

const float temp_calibration = -0.05; // this value is a complete ballpark.

int raw;
float volts, temp;

float temp_low = 22;  // lower bound of preferred temperature
float temp_high = 25; // upper bound of preferred temperature
float range = 1;      // difference in degrees between when an actuator will be turned off and on

void setup() {
  Serial.begin(9600);

  pinMode(PIN_LED_HEAT, OUTPUT);
  pinMode(PIN_LED_FAN, OUTPUT);
  pinMode(PIN_TEMP, INPUT);
}

void loop() {
  raw = analogRead(PIN_TEMP);
  Serial.print("Raw: ");
  Serial.print(raw);

  volts = (float) (raw * (3.3/1023.0));
  volts += temp_calibration;
  Serial.print("\tVolts: ");
  Serial.print(volts);

  temp = volts * 100;
  Serial.print("\tTemp: ");
  Serial.print(temp);
  Serial.print("°C");

  Serial.println();

  if (temp > temp_high + range) {
    set_fan(true);
  }
  if (temp < temp_high - range) {
    set_fan(false);
  }
  if (temp < temp_low - range) {
    set_heater(true);
  }
  if (temp > temp_low + range) {
    set_heater(false);
  }
}

void set_fan(bool state) {
  // if state = true -> turn fan on
  // if state = false -> turn fan off
  // fan state temporarily symbolized by a green LED
  digitalWrite(PIN_LED_FAN, state);
}

void set_heater(bool state) {
  // if state = true -> turn heater on
  // if state = false -> turn heater off
  // fan state temporarily symbolized by a red LED
  digitalWrite(PIN_LED_HEAT, state);
}