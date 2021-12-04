#define LDR A0

int cal_value;
int light = 0; 


void setup() {
  pinMode(LDR, INPUT);
  Serial.begin(9600);
  Serial.println("Resistor values:");
}

void loop() {
  light = analogRead(LDR);
  if(light < 800){
    Serial.println("LOW");
    }
  else{
    Serial.println("HIGH");
    }
  delay(200);
}
