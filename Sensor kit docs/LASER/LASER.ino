#define LASERPIN 13
#define PHOTORECEIVER A1

int light = 0;
void setup() 
{
//  pinMode (LASERPIN, OUTPUT);  //define laser POWER pin as D2
  pinMode (PHOTORECEIVER, INPUT); //define status ON/OFF
  Serial.begin(9600);
}

void loop() 
{
//  digitalWrite(LASERPIN,HIGH);  //open laser head
//  delay(1000);
//  digitalWrite(LASERPIN,LOW);  //close laser head
  delay(100);
  light = analogRead(PHOTORECEIVER);
  Serial.println(light,DEC);
}
