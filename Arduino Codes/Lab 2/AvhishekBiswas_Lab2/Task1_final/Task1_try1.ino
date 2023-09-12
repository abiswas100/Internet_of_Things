// LED definitions in the datasheet
// D13 (PIN_LED_13): Blue
// RX (PIN_LED_RXL): Yellow

// Variables will change:
long previousMillisBlue = 0;  // will store the last time Blue LED was updated
long previousMillisYellow = 0;  // will store the last time Yellow LED was updated

// intervals at which to blink (milliseconds)
long intervalBlue = 500;  
long intervalYellow = 1000; 

void setup() {
  // set the digital pins as output:
  pinMode(PIN_LED_13, OUTPUT);
  pinMode(PIN_LED_RXL, OUTPUT);
  SerialUSB.begin(9600);
}

void loop() {
  // here is where you'd put code that needs to be running all the time.
  unsigned long currentMillis = millis();

  // Check if it's time to toggle the Blue LED
  if (currentMillis - previousMillisBlue > intervalBlue) {
    previousMillisBlue = currentMillis;
    if (digitalRead(PIN_LED_13) == LOW) {
      digitalWrite(PIN_LED_13, HIGH);
      SerialUSB.println("Blue LED is on");
    } else {
      digitalWrite(PIN_LED_13, LOW);
      SerialUSB.println("Blue LED is off");
    }
  }

  // Check if it's time to toggle the Yellow LED
  if (currentMillis - previousMillisYellow > intervalYellow) {
    previousMillisYellow = currentMillis;
    if (digitalRead(PIN_LED_RXL) == LOW) {
      digitalWrite(PIN_LED_RXL, HIGH);
      SerialUSB.println("Yellow LED is on");
    } else {
      digitalWrite(PIN_LED_RXL, LOW);
      SerialUSB.println("Yellow LED is off");
    }
  }
}
