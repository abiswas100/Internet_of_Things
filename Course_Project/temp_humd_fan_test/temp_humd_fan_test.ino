#include <dht11.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHT11PIN 4
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
dht11 DHT11;

float humidity = 0;
float dht_temperature = 0;
float ds_temperatureC=0;


const int fanPin = 3;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
  sensors.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  int chk = DHT11.read(DHT11PIN);
  humidity = DHT11.humidity;
  Serial.print("Humidity (%) = "); 
  Serial.println(humidity);
  dht_temperature = DHT11.temperature;
  Serial.print("DHT_Temperature  (C) = "); 
  Serial.println(dht_temperature);

//  sensors.requestTemperatures(); 
//  
//  // Get the temperature in Celsius
//  ds_temperatureC = sensors.getTempCByIndex(0);
//  
//  // Check if reading is valid
//  if (ds_temperatureC != DEVICE_DISCONNECTED_C) {
//    // Display temperature on Serial Monitor
//    Serial.print("ds18b20_Temperature: ");
//    Serial.print(ds_temperatureC);
//    Serial.println(" °C");
//  } else {
//    Serial.println("Error: Could not read temperature data");
//  }

  if (dht_temperature >= 24 || humidity >= 60){
    digitalWrite(fanPin, HIGH);
//    fanState = true;
//    timer.tick();    
  }
  else{
    digitalWrite(fanPin, LOW);
  }
  delay(500);
}
