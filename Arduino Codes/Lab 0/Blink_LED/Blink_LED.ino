void setup() {
  SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);
}

void loop() {
  digitalWrite(PIN_LED_13, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_13, LOW);
  delay(1000);
}
