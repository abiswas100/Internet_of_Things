#include<avr/wdt.h>
#include <dht11.h>
#include <OneWire.h>
#include <DallasTemperature.h>
//#include <arduino-timer.h>

unsigned long prevtime;
unsigned long currenttime;

// Sensor signal pins
const int soilPin = A0;//Declare a variable for the soil moisture sensor
int soilPower = 10;
const int doorPin = 7;
const int photoPin = A2;
#define DHT11PIN 4
#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
dht11 DHT11;

//Sensor reading variables
int soilMoisture_val = 0; //value for storing moisture value 
int doorState = 0;
float humidity = 0;
float dht_temperature = 0;
float ds_temperatureC=0;
float photoVal= 0;

// Controll pins

const int fanPin = 3;
const int lightPin = 13;
const int pumpPin = 6;
const int buzzerPin = 12;

//auto timer = timer_create_default(); // create a timer with default settings

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); /* Define baud rate for serial communication */
  Serial.println("Watchdog Demo Starting");
  wdt_disable();  /* Disable the watchdog and wait for more than 2 seconds */
  delay(3000);  /* Done so that the Arduino doesn't keep resetting infinitely in case of wrong configuration */
  wdt_enable(WDTO_2S);  
  /* Enable the watchdog with a timeout of 2 seconds */

  pinMode(soilPower, OUTPUT);//Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW);//Set to LOW so no power is flowing through the sensor
  pinMode(doorPin, INPUT_PULLUP); // set arduino pin to input pull-up mode

  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  
  
  sensors.begin();
  prevtime = millis();

//  timer.at(1000, toggle_led);
}

void loop() {
  // put your main code here, to run repeatedly:
  currenttime = millis();
  //get soil moisture value from the function below and print it
  if (currenttime - prevtime >= 2000){
    currenttime = millis();
    Serial.print("Time = ");
    Serial.println(currenttime);
    readMoisture();

    currenttime = millis();
    Serial.print("Time = ");
    Serial.println(currenttime);
  
    readdoorState();

    currenttime = millis();
    Serial.print("Time = ");
    Serial.println(currenttime);
  
    readDHT();

    currenttime = millis();
    Serial.print("Time = ");
    Serial.println(currenttime);
    
//    ds18b20_temp();

    readLight();
    
    wdt_reset();
    prevtime = millis();
  }
  wdt_reset();
}


void readMoisture(){
  digitalWrite(soilPower, HIGH);//turn D7 "On"
//  delay(10);//wait 10 milliseconds 
  soilMoisture_val = analogRead(soilPin);//Read the SIG value form sensor 
  digitalWrite(soilPower, LOW);//turn D7 "Off"
  Serial.print("Soil Moisture = ");
  Serial.println(soilMoisture_val);
  if (soilMoisture_val >= 626){
    digitalWrite(pumpPin, LOW);       
  }
  else{
    digitalWrite(pumpPin, HIGH);
    Serial.println("Dry soil! turning PUMP ON");
  }
  
}

int readdoorState(){
  doorState = digitalRead(doorPin);
  Serial.print("Door State = ");
  if (doorState) { 
    Serial.println("Opened");
    digitalWrite(buzzerPin, HIGH);
  }
  else {
    Serial.println("Closed");
    digitalWrite(buzzerPin, LOW);    
  }
}

void readDHT(){
  int chk = DHT11.read(DHT11PIN);
  humidity = DHT11.humidity;
  dht_temperature = DHT11.temperature;
//  Serial.print("Humidity (%): ");
//  Serial.println((float)DHT11.humidity, 2);
//
//  Serial.print("Temperature  (C): ");
//  Serial.println((float)DHT11.temperature, 2);
  Serial.print("Humidity (%) = "); 
  Serial.println(humidity);
  Serial.print("DHT_Temperature  (C) = "); 
  Serial.println(dht_temperature);

  if (dht_temperature >= 23 || humidity >= 70){
    digitalWrite(fanPin, HIGH); 
    Serial.println("Too hot! turning FANS ON");  
  }
  else{
    digitalWrite(fanPin, LOW);
  }
}

//void ds18b20_temp(){
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
//
//  if (ds_temperatureC >= 29 || humidity >= 60){
//    digitalWrite(fanPin, HIGH);
////    fanState = true;
////    timer.tick();    
//  }
//  else{
//    digitalWrite(fanPin, LOW);
//  }
//}

void readLight(){
  photoVal = analogRead(photoPin);//Read the voltage value from sensor 
  Serial.print("Light intensity = ");
  Serial.println(photoVal);
  if (photoVal >= 390){
    digitalWrite(lightPin, LOW);   
  }
  else{
    digitalWrite(lightPin, HIGH);
    Serial.println("Night Time! turning LIGHTS ON");
  }
}
