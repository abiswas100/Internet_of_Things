const int photoPin = A2;
float photoVal= 0;
const int lightPin = 13;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(lightPin, OUTPUT);
  digitalWrite(lightPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  photoVal = analogRead(photoPin);//Read the voltage value from sensor 
  Serial.print("Light intensity = ");
  Serial.println(photoVal);
  delay(1000);
  if (photoVal >= 500){
    digitalWrite(lightPin, LOW);   
  }
  else{
    digitalWrite(lightPin, HIGH);
  }
  }
