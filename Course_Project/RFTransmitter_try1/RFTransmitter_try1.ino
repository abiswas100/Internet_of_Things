#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
const int ledPin = 9; // LED connected to digital pin 9

void setup() {
  pinMode(ledPin, OUTPUT); // Initialize the LED pin as an output
  Serial.begin(9600); // Start serial communication at 9600 baud rate

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
}

void loop() {
  const char text[] = "Hello World";

  digitalWrite(ledPin, HIGH); // Turn the LED on
  radio.write(&text, sizeof(text)); // Send the message
  // delay(500);
  digitalWrite(ledPin, LOW); // Turn the LED off

  Serial.println("Message sent"); // Print message to serial monitor
}
