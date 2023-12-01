/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-door-sensor
 */

const int DOOR_SENSOR_PIN = 7; // Arduino pin connected to door sensor's pin

int doorState;

void setup() {
  Serial.begin(9600);                     // initialize serial
  pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP); // set arduino pin to input pull-up mode
}

void loop() {
  doorState = digitalRead(DOOR_SENSOR_PIN); // read state
  delay(1000);
  Serial.println(doorState);
//  if (doorState == 1) {
//    Serial.println("The door is open");
//  } else {
//    Serial.println("The door is closed");
//  }
}
