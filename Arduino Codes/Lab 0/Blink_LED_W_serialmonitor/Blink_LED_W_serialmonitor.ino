void setup() {
  //SerialUSB.begin(9600);
  pinMode(PIN_LED_13, OUTPUT);
}

void loop() {
  digitalWrite(PIN_LED_13, HIGH);
  SerialUSB.println("Built_in LED is on");
  delay(2000);
  digitalWrite(PIN_LED_13, LOW);
  SerialUSB.println("Built_in LED is off");
  delay(2000);
}

