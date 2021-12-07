#include <Servo.h>
/* PINOUT SELECTION   --------------------------------------*/
    #define LDR A1
    #define SIGNAL_TO_ESP 2
    #define BUZZER 3        //PWM WE CAN TUNE THE FREQUENCY!!
    #define Servo_pin 6//9
    
    #define stepMotorPin1 2 //10
    #define stepMotorPin2 3 //11
    #define stepMotorPin3 4 //12
    #define stepMotorPin4 5 //13

    #define SEL0 4 //
    #define SEL1 5 // ----------- FOR UNO  
    
    /* CALIBRATION CONSTANTS FOR STEPPER   -----------------------*/
    #define Time_interval_stepper 1000
    #define Time_interval_servo 500
    #define stepper_steps 50
    
    
    /* CALIBRATION CONSTANTS FOR PHOTOINTERRUPTER -----------------*/
    #define PHOTOVOLTAGE_threshold 900
    #define Delay_time_photoint 50
    #define TIMEOUT_COUNTER 10000
    #define Time_between_detections 100          //to set around 100ms ?
    
    int mode, operation_over = 1;
    
    
/*        GLOBAL VARIABLES FOR ENGINES    */
    Servo Servo1;   // create Servo object
    
    int cycle_stage;
    int motor_Speed = 4;
    int engine_over = 0;

/*         GLOBAL VARIABLES FOR verify_success FUNCTION       */
    int attempts = 0;
    int success = 3;

/*        PHOTOINTERRUPTER GLOBAL VARIABLES        */
    unsigned long t_current, t_0;
    int counter;
    int mytime[20];   //size can be changed
    int i=1;
    int calib_val = 900;
    int error_count = 0;
    int check_time_elapsed_success =0;
    boolean start_1 = 1;


void setup()
{
    setup_pins();

    Serial.begin(9600); 
    delay(2000); 

    Serial.println("serial Begin!\n");
}



void loop() 
{
   //       delay(500);   //IF DELAY IS ABOVE 50ms Pill likely to not be detected :(
  
  /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIMESTAMP TO CURRENT TIMESTAMP  */
  t_current = millis();

/*          WHENEVER FLAG operation_over is not set to 1 ->                                */
  
  if (operation_over == 1)
  {

      choose_to_dispense();
      digitalWrite(LED_BUILTIN, LOW);
  
  }          
    else
    {
        //later ADD BUTTON TO PRESS TO STOP BUZZ AND ACTIVATE PILL!

        //dispense_pills activates the engines

        dispense_pills();
        if ((cycle_stage == 2 || cycle_stage == 3) && success!=0)
        {   
            success = photointerrupter();              //success takes either 0 , 1, 2  for failure, success, no pill dispensed 
        }

        if (engine_over == 1) {verify_success();}  //menu_var = 0;}


    }
}



//******************          FUNCTIONS             **************************


void choose_to_dispense()
{
    Serial.println("Do you want to start the dispensing test?\n  'Y' for YES or 'N' for NO ");
    //while (Serial.available() == 0) {}  Serial.setTimeout(500);   //This sets the maximum time to wait for serial data from user.
    String menuChoice = "Y";//Serial.readString();
    Serial.print(menuChoice);    Serial.println("\n------------------------------------------------------------");
    if ((menuChoice == "Y") || (menuChoice == "y"))                                                    
    {
        selector_function();  //We activate the correct module to be operated!
                  
        operation_over = 0;  //WE START AN OPERATION, BYPASS MENU
        engine_over = 0;     //WE ALLOW DISPENSE PILLS TO OPERATE ONCE WITH THIS FLAG
                  
        Serial.println("Start of operation!");;  //add module number to be passed to refill_module       ------------                        TO DO
    }

}


void selector_function()
{
  int schedule_enumerator;
  int module_number[4] = {1, 2, 3, 4};

  Serial.print("Please insert the module number to activate! ");
  while (Serial.available() == 0) {}  schedule_enumerator = Serial.parseInt();

  switch (module_number[schedule_enumerator]) {
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


int photointerrupter()
{
    if(start_1)
    {
      delay(Delay_time_photoint); 
      start_1 = 0;   
    }  
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
        Serial.println("ERROR when dispensing pill!!"); delay(1000);
        //send_email();

        break;
      case 1:                                //SUCCESS!
        operation_over = 1;
        Serial.println("Pill dispensed successfully!!"); delay(1000);
        break;
      case 2:                                 //TRY AGAIN!   NO PILL WAS DISPENSED
        Serial.println("Reattempting pill dispensing!"); delay(1000);
        
        //attempts++; Serial.print("\t attempt number:"); Serial.println(attempts);
        //if(attempts >= 2){ operation_over = 1; attempts = 0; }
        break;  
      default:
        Serial.println("Default!");     //-------------------------------------
  }

}



void dispense_pills()
{
  if (engine_over != 1)
  {
      if((t_current - t_0 >1000) &&  (cycle_stage == 1))                                            //   STAGE 1
      {       
          lock_pill();
          t_0 = t_current;
          cycle_stage = 2;
          Serial.println("lock pill! Stage 1");
      }


/*                ESSENTIAL FOR PHOTOINTERRUPTER TO WORK HERE                        */

      if((t_current - t_0 >1000) &&  cycle_stage == 2)   //wait 500ms                             //   STAGE 2
      {   
          open_gate();
          t_0 = t_current;
          cycle_stage = 3;
          Serial.println("open gate! Stage 2");
      }
  

      if((t_current - t_0 >2000) && cycle_stage == 3)   //wait 2000ms                             //   STAGE 3
      {   
          close_gate();
          t_0 = t_current;
          cycle_stage = 4;
          Serial.println("close gate! Stage 3");
      }


      if((t_current - t_0 >1000) && cycle_stage == 4 )  //wait 500ms                             //   STAGE 3
      {   
          unlock_pill();
          t_0 = t_current;
          cycle_stage = 1;  engine_over = 1;                                //SETS THE END OF THE ENGINE OPERATION!!
          Serial.println("unlock pill! Stage 4");    
      }
  }
}




void unlock_pill()   // Stepper motor operation
{
    for(int j=0; j<stepper_steps; j++){
        
        int motor_Speed = 4;   /*  TO BE INSERTED THE RIGHT motor speed value   */ 
        digitalWrite(stepMotorPin4, HIGH);
        digitalWrite(stepMotorPin3, LOW);
        digitalWrite(stepMotorPin2, LOW);
        digitalWrite(stepMotorPin1, LOW);
        delay(motor_Speed);
        digitalWrite(stepMotorPin4, LOW);
        digitalWrite(stepMotorPin3, HIGH);
        digitalWrite(stepMotorPin2, LOW);
        digitalWrite(stepMotorPin1, LOW);
        delay(motor_Speed);
        digitalWrite(stepMotorPin4, LOW);
        digitalWrite(stepMotorPin3, LOW);
        digitalWrite(stepMotorPin2, HIGH);
        digitalWrite(stepMotorPin1, LOW);
        delay(motor_Speed);
        digitalWrite(stepMotorPin4, LOW);
        digitalWrite(stepMotorPin3, LOW);
        digitalWrite(stepMotorPin2, LOW);
        digitalWrite(stepMotorPin1, HIGH);
        delay(motor_Speed);
    }

}

void lock_pill()   //Stepper_motor operation
{
    for(int j=0; j<stepper_steps; j++){
        
        int motor_Speed = 4;   /*  TO BE INSERTED THE RIGHT motor speed value   */ 
        digitalWrite(stepMotorPin4, LOW);
        digitalWrite(stepMotorPin3, LOW);
        digitalWrite(stepMotorPin2, LOW);
        digitalWrite(stepMotorPin1, HIGH);
        delay(motor_Speed);
        digitalWrite(stepMotorPin4, LOW);
        digitalWrite(stepMotorPin3, LOW);
        digitalWrite(stepMotorPin2, HIGH);
        digitalWrite(stepMotorPin1, LOW);
        delay(motor_Speed);
        digitalWrite(stepMotorPin4, LOW);
        digitalWrite(stepMotorPin3, HIGH);
        digitalWrite(stepMotorPin2, LOW);
        digitalWrite(stepMotorPin1, LOW);
        delay(motor_Speed);
        digitalWrite(stepMotorPin4, HIGH);
        digitalWrite(stepMotorPin3, LOW);
        digitalWrite(stepMotorPin2, LOW);
        digitalWrite(stepMotorPin1, LOW);
        delay(motor_Speed);
    }
}


void open_gate()
{   
    int motor_Speed = 1000;
    Servo1.write (40);

}
void close_gate()
{
    int motor_Speed = 1000;
    Servo1.write (140);
}




void setup_pins()
{
    pinMode(LDR, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);    

    Servo1.attach (Servo_pin); //Il Servo1 è collegato al pin digitale 
    pinMode(stepMotorPin1, OUTPUT);
    pinMode(stepMotorPin2, OUTPUT);
    pinMode(stepMotorPin3, OUTPUT);
    pinMode(stepMotorPin4, OUTPUT);

    /* SETUP Send_email() */
    pinMode(SIGNAL_TO_ESP, OUTPUT); digitalWrite(SIGNAL_TO_ESP, LOW);


        /* SETUP ENGINES TIMING */
    t_0 = millis(); cycle_stage = 1;

    /*SETUP PHOTOINTERRUPTER*/
    mytime[0] = t_0;   //Serial.print("mytime[0]: "); Serial.println(mytime[i]);
    i++;
}
