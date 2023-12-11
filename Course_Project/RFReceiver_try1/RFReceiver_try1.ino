#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
const int ledPin = 10; // LED connected to digital pin 9

void setup() {
  pinMode(ledPin, OUTPUT); // Set the LED pin as an output
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);

    digitalWrite(ledPin, HIGH); // Turn the LED on
    delay(500);                // Keep it on for 500 milliseconds
    digitalWrite(ledPin, LOW); // Turn the LED off
  }
}
