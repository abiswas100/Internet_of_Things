const int yellowLED = 2;  // Connect your yellow LED here
const int blueLED = 13;   // Built-in blue LED

unsigned long yellowPreviousMillis = 0;
unsigned long bluePreviousMillis = 0;
const long yellowInterval = 1000;  // 1 second
const long blueInterval = 500;     // 0.5 second

void setup() {
  pinMode(yellowLED, OUTPUT);
  pinMode(blueLED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Yellow LED
  if (currentMillis - yellowPreviousMillis >= yellowInterval) {
    yellowPreviousMillis = currentMillis;
    if (digitalRead(yellowLED) == LOW) {
      digitalWrite(yellowLED, HIGH);
      SerialUSB.println("Yellow LED is on.");
    } else {
      digitalWrite(yellowLED, LOW);
      SerialUSB.println("Yellow LED is off.");
    }
  }

  // Blue LED
  if (currentMillis - bluePreviousMillis >= blueInterval) {
    bluePreviousMillis = currentMillis;
    if (digitalRead(blueLED) == LOW) {
      digitalWrite(blueLED, HIGH);
      SerialUSB.println("Blue LED is on.");
    } else {
      digitalWrite(blueLED, LOW);
      SerialUSB.println("Blue LED is off.");
    }
  }
}



