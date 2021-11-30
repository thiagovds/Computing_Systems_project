#define SEL0 21 //BLUE CABLE
#define SEL1 20 //PURPLE CABLE

int module_number = 0;

void setup() {
  pinMode(SEL0, OUTPUT);
  pinMode(SEL1, OUTPUT);

  Serial.begin(9600);
  Serial.println("\t serial started!");
}

void loop() 
{
  Serial.println("Select the module number to be selected");
  while (Serial.available() == 0) {}  module_number = Serial.parseInt();
  selector_function();

}

void selector_function()
{
  switch (module_number) {
      case 1:
        digitalWrite(SEL0, LOW);   //00
        digitalWrite(SEL1, LOW);
        break;
      case 2:
        digitalWrite(SEL0, HIGH);   //01
        digitalWrite(SEL1, LOW);
        break;
      case 3:
        digitalWrite(SEL0, LOW);   //10
        digitalWrite(SEL1, HIGH);
        break;
      case 4:
        digitalWrite(SEL0, HIGH);   //11
        digitalWrite(SEL1, HIGH);
        break;        
      default:
        Serial.println("please insert a valid input!!");
  }
}
