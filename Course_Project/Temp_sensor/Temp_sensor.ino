#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  // Start serial communication
  Serial.begin(9600);
  
  // Initialize the DS18B20 sensor
  sensors.begin();
}

void loop() {
  // Request temperature conversion
  sensors.requestTemperatures(); 
  
  // Get the temperature in Celsius
  float temperatureC = sensors.getTempCByIndex(0);
  
  // Check if reading is valid
  if (temperatureC != DEVICE_DISCONNECTED_C) {
    // Display temperature on Serial Monitor
    Serial.print("Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
  } else {
    Serial.println("Error: Could not read temperature data");
  }
  
  // Delay before taking the next reading
  delay(2000); // Adjust the delay time as needed
}
