
// Things needed to be done in this code .. 

// Install the sensor packages.
// Add code for threshold to light up a certain LED in analog pin X to start signal system for each sensor.
// Keep logs of all these events.
// Save the data in a CSV to train an ML Model.

//#include <DHT.h>  // Include the library for the DHT sensor

#define DHTPIN 2     // Define the pin where the DHT sensor is connected
#define DHTTYPE DHT11 // Define the type of DHT sensor
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_MOISTURE_PIN A0 // Define the pin for the soil moisture sensor
#define LIGHT_SENSOR_PIN A1  // Define the pin for the light sensor
#define CO2_SENSOR_PIN A2    // Define the pin for the CO2 sensor


void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read soil moisture
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  // Read light level
  int lightLevel = analogRead(LIGHT_SENSOR_PIN);

  // Read CO2 level
  int co2Level = analogRead(CO2_SENSOR_PIN);

  // Check if the sensor readings are valid and assign random values if not
  if (isnan(temperature)) {
    temperature = random(20, 30); // Random temperature between 20 and 30
  }
  if (isnan(humidity)) {
    humidity = random(30, 70); // Random humidity between 30% and 70%
  }
  if (soilMoisture == 0) {
    soilMoisture = random(300, 700); // Random soil moisture value
  }
  if (lightLevel == 0) {
    lightLevel = random(200, 800); // Random light level
  }
  if (co2Level == 0) {
    co2Level = random(400, 1000); // Random CO2 level
  }

  // Print the sensor values
  Serial.print("Temperature: "); Serial.println(temperature);
  Serial.print("Humidity: "); Serial.println(humidity);
  Serial.print("Soil Moisture: "); Serial.println(soilMoisture);
  Serial.print("Light Level: "); Serial.println(lightLevel);
  Serial.print("CO2 Level: "); Serial.println(co2Level);

  delay(2000); // Wait for 2 seconds before reading again
}
