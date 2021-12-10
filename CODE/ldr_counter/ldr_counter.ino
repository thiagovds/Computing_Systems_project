/*        PHOTOINTERRUPTER GLOBAL VARIABLES        */
    int counter;
    int mytime[20];   //size can be changed
    int i=1;
    int calib_val = 900;
    int error_count = 0;
    int check_time_elapsed_success =0;

/*         GLOBAL VARIABLES FOR verify_success FUNCTION       */
    int attempts = 0;
    int success = 0;



// PLEASE RUN new file ldr_counter_and_engine.ino for testing and calibrating detection of pill.


/*-------------------------------------PREVIOUS CODE------------------------------------------*/

#define LDR A0
#define Delay_time_photoint 15//              CALIBRATE DELAY HERE

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
    delay(Delay_time_photoint);    //IF DELAY IS ABOVE 50ms Pill likely to not be detected :(     *************************************************************


  }
}



/*----------------------------------------FUNCTIONS--------------------------------------------*/




/*

int photointerrupter()
{
    delay(Delay_time_photoint);  
    int light = analogRead(LDR);

    if (light < calib_val)                      //detection of light beam obstruction
    {
        Serial.println(light); 
        counter++; mytime[i] = millis();          //record passing of pill and its timestamp
    
        if(mytime[i] - mytime[i-1] < Time_between_detections   &&   i != 0)   // if 2 pill counting events are less than 100ms apart, that means that 2 or more pills were dispensed at once!
        {
            Serial.println("DISPENSING ERROR! 2 or more pills dispensed!"); 

            //send_email();              // SEND EMAIL!!!!!!           -------------------------------------------------------------------------------
            
            Serial.print("counter : ");           Serial.println(counter);    
            Serial.print("time difference : ") ;  Serial.println(mytime[i]-mytime[i-1]);
            Serial.print("mytime[i] : ") ;        Serial.println(mytime[i]);
            Serial.print("mytime[0] : ") ;        Serial.println(mytime[i-1]);  
            Serial.print("i = ");                 Serial.println(i);

            i=0; mytime[i] = millis();  //reset mytime  
            i++;  error_count++; check_time_elapsed_success =0;

            return 0;    //return to success variable on main
        }
        else
        {
            Serial.print("counter : ");             Serial.println(counter);    
            Serial.print("time difference : ") ;    Serial.println(mytime[i]-mytime[i-1]);
            Serial.print("mytime[i] : ") ;          Serial.println(mytime[i]);
            Serial.print("mytime[0] : ") ;          Serial.println(mytime[i-1]);
            Serial.print("i = ");                   Serial.println(i);
            i++; check_time_elapsed_success=1;

        }

    }

    else if (t_current - mytime[i] > TIMEOUT_COUNTER  &&  (i != 0  && counter==0))   //     CASE THAT NO PILL IS DISPENSED.    ----------------------------------
    {
        Serial.println("TIMEOUT FOR PILL DETECTION! \t NO PILL DISPENSED");  //TO ADD! TRY AGAIN TO DISPENSE PILL!
        Serial.print("current time - last event time > TIMEOUT = "); Serial.println(t_current - mytime[i]);
        Serial.print("t_current: ") ;          Serial.println(t_current);
        Serial.print("mytime["); Serial.print(i); Serial.print("] : ");  Serial.println(mytime[i]);

        return 2;     //return to success variable for verify_success
    }
    else if (t_current - mytime[i] > TIMEOUT_COUNTER && (error_count==0 && check_time_elapsed_success==1)) //     CASE THAT ONLY 1 PILL IS DISPENSED.    ----------------------------------
    {
        return 1;
    }


}


void verify_success()
{

  switch (success) {
      case 0:                                //ERROR!
        operation_over = 1;
        Serial.println("ERROR when dispensing pill!!");
        delay(2000);
        send_email();

        break;
      case 1:                                //SUCCESS!
        operation_over = 1;
        Serial.println("Pill dispensed successfully!!");
        delay(2000);
        break;
      case 2:                                 //TRY AGAIN!   NO PILL WAS DISPENSED
        Serial.println("Reattempting pill dispensing!");
        delay(2000);
        attempts++; Serial.print("\t attempt number:"); Serial.println(attempts);
        if(attempts >= 2){ operation_over = 1; attempts = 0; }

        break;  
      default:
        Serial.println("Default!");     //-------------------------------------
  }

}



*/