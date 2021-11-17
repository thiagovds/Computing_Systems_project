#define LDR A1


int calib_val = 900;   //calibration value, decrease if false counts
int light = 0;
int counter = 0;
int error_count = 0;
unsigned long mytime[] = {0,0,0};
int i = 0;
void setup() 
{
  pinMode (LDR, INPUT); //define status ON/OFF
  Serial.begin(9600);     Serial.println("Serial started");
  mytime[0] = millis();   Serial.println(mytime[i]);
  i++;
}

void loop() 
{
  delay(15);  light = analogRead(LDR);

  if (light < calib_val)                      //detection of light beam obstruction
  {
    counter++; mytime[i] = millis();          //record passing of pill and its timestamp
    
    if(mytime[i] - mytime[i-1] < 1000   &&   i != 0)   // if 2 pill counting events are less than 100ms apart, that means that 2 or more pills were dispensed at once!
    {
        Serial.println("DISPENSING ERROR! 2 or more pills dispensed!"); 

        //ADD LINE HERE TO SEND EMAIL!!!!!!           -------------------------------------------------------------------------------

        i=0; mytime[i] = millis();  delay(4000);  //4 seconds for user read and reset mytime  
        error_count++;
        
        Serial.print("counter : ");           Serial.println(counter);    
        Serial.print("time difference : ") ;  Serial.println(mytime[i]-mytime[i-1]);
        Serial.print("mytime[i] : ") ;        Serial.println(mytime[i]);
        Serial.print("mytime[0] : ") ;        Serial.println(mytime[i-1]);  
        Serial.print("i = ");                 Serial.println(i);   
    }
    else
    {
      Serial.print("counter : ");             Serial.println(counter);    
      Serial.print("time difference : ") ;    Serial.println(mytime[i]-mytime[i-1]);
      Serial.print("mytime[i] : ") ;          Serial.println(mytime[i]);
      Serial.print("mytime[0] : ") ;          Serial.println(mytime[i-1]);
      Serial.print("i = ");                   Serial.println(i);   
    }
    
    
    
    
  
    
    
  
    
    i++;
    if (counter >= 100){counter = 0;}
  }
  else
  {
    //  Serial.print("NO COUNT\n light =");
    //  Serial.println(light);            
  }
}
