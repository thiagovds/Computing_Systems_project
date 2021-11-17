#define LDR A1


int calib_val = 900;   //calibration value, decrease if false counts
int light = 0;
int counter = 0;
int error_count = 0;
int long mytime[3] = {0,0,0};
int i = 0;
void setup() 
{
  Serial.println(mytime[i]);
  pinMode (LDR, INPUT); //define status ON/OFF
  Serial.begin(9600);
  Serial.println("Serial started");
}

void loop() 
{
  delay(15);  light = analogRead(LDR);

  if (light < calib_val)                      //detection of light beam obstruction
  {
    counter++; mytime[i] = millis();          //record passing of pill and its timestamp
    
    if(mytime[i] - mytime[0] < 1000   &&   i != 0)   // if 2 pill counting events are less than 100ms apart, that means that 2 or more pills were dispensed at once!
    {
        Serial.println("DISPENSING ERROR! 2 or more pills dispensed!"); 

        //ADD LINE HERE TO SEND EMAIL!!!!!!           -------------------------------------------------------------------------------

        Serial.print("counter : ");
        Serial.println(counter);    
        Serial.print("time difference : ") ;Serial.println(mytime[i]-mytime[0]);
        
        Serial.print("mytime[i] : ") ;Serial.println(mytime[i]);
        Serial.print("mytime[0] : ") ;Serial.println(mytime[0]);
        
        Serial.print("i = ");Serial.println(i);   

        delay(5000);  //5 seconds for user read  
        error_count++;
    }
    else if (i == 2) { Serial.println(i); i = 0;}

    Serial.print("counter value is : ");
    Serial.println(counter, DEC);

    Serial.print("photovoltage : ");
    Serial.println(light,DEC);

        Serial.print("time difference : ") ;Serial.println(mytime[i]-mytime[0]);
        
        Serial.print("mytime[i] : ") ;Serial.println(mytime[i]);
        Serial.print("mytime[0] : ") ;Serial.println(mytime[0]);
        
        Serial.print("i = ");Serial.println(i);   
    
  if (counter >= 100){counter = 0;}
  i++;
  }
  else
  {
  //  mytime[] = millis();
  //  Serial.print("NO COUNT\n light =");
  //  Serial.println(light);    
  //  Serial.println(mytime[i]-mytime[0]);
  //  Serial.print("i = ");Serial.println(i);        
  }
}
