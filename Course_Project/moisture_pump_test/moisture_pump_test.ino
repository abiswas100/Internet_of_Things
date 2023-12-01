const int soilPin = A0;//Declare a variable for the soil moisture sensor
int soilPower = 10;
int soilMoisture_val = 0;
const int pumpPin = 6;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600); 
pinMode(soilPower, OUTPUT);//Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(soilPower, HIGH);//turn D7 "On"
//  delay(10);//wait 10 milliseconds 
  soilMoisture_val = analogRead(soilPin);//Read the SIG value form sensor 
  digitalWrite(soilPower, LOW);//turn D7 "Off"
  Serial.print("Soil Moisture = ");
  Serial.println(soilMoisture_val);
  if (soilMoisture_val >= 800){
    digitalWrite(pumpPin, LOW);   
  }
  else{
    digitalWrite(pumpPin, HIGH);
  }
}
