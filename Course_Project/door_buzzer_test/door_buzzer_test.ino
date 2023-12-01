const int doorPin = 8;
int doorState = 0;
const int buzzerPin = 12;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(buzzerPin, OUTPUT);
  pinMode(doorPin, INPUT_PULLUP); 
  digitalWrite(buzzerPin, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
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
  delay(1000);
//  digitalWrite(buzzerPin, HIGH);
  }
