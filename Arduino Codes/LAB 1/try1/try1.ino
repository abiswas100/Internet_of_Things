void setup() {
  SerialUSB.begin(9600); // Set the baud rate to 9600
}

void loop() {
  SerialUSB.println("Hello, world!");
  // delay(1000); // Wait for 1 second
}
