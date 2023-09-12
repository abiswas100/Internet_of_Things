// LED definitions in the datasheet
// D13 (PIN_LED_13): Blue
// TX (PIN_LED_TXL): Green
// RX (PIN_LED_RXL): Yellow
// Variables will change:
long previousMillis = 0; // will store the last time LED was updated
// the following variable is a long because the time, measured in milliseconds,
// will quickly become a bigger number than can be stored in an int.
long interval = 1000; // interval at which to blink (milliseconds)
void setup() {
// set the digital pin as output:
pinMode(PIN_LED_13, OUTPUT);
}
void loop()
{
// here is where you'd put code that needs to be running all the time.
unsigned long currentMillis = millis();
if(currentMillis - previousMillis > interval) {
  // save the last time you blinked the LED
  // Serial.println("Printing ??");
previousMillis = currentMillis;
// Do something here
}
}