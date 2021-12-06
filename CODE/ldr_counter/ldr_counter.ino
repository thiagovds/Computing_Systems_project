#define LDR A0

int cal_value = 1005; //IMPORTANT: STILL NOT PICKING THE PILL PASSING
int light = 0; 
int counter = 0;

void setup() {
  pinMode(LDR, INPUT);
  Serial.begin(9600);
  Serial.println("Pill dispensed: ");
}

void loop() {
  
  light = analogRead(LDR);
  
  if(light < cal_value){
    counter = counter + 1;
    Serial.println(counter);
    delay(500);
  }
}
