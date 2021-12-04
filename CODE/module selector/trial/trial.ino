void setup() {
  
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  Serial.begin(9600);
  Serial.println("press 1 for 00");
  Serial.println("press 2 for 11");

}

void loop() {
  
  digitalWrite(2, HIGH);
  digitalWrite(3, HIGH);
  delay(2000);
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  delay(2000);
  
}
