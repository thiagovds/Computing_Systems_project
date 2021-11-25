#include <Servo.h> 

/* PINOUT SELECTION   --------------------------------------*/
#define LDR A1
#define SIGNAL_TO_ESP 2
#define Servo_pin 9



/* CALIBRATION CONSTANTS FOR STEPPER   -----------------------*/
#define Time_interval_stepper 1000
#define Time_interval_servo 500
#define stepper_steps 50

#define stepMotorPin1 10
#define stepMotorPin2 11
#define stepMotorPin3 12
#define stepMotorPin4 13

/* CALIBRATION CONSTANTS FOR PHOTOINTERRUPTER -----------------*/
#define Delay_time_photoint 20
#define TIMEOUT_COUNTER 10000
#define Time_between_detections 1000          //to set around 100ms ?


/*  INITIALIZATION OF GLOBAL VARIABLES FOR MAIN  ----------------------  */

Servo Servo1;

int cycle_stage;
int start = 1;

int mode, operation_over = 1;
int motor_Speed = 4;
int engine_over = 0;

int Schedule_time[][2];
int module_number;


unsigned long t_current, t_0;

int attempts = 0;

//------- ADD PILLS global variables
String medicine;
int number_of_pills;

int success = 0;

/*-----------------------*/

//-------------PHOTOINTERRUPTER GLOBAL VARIABLES --
int counter;
int mytime[20];
int i=1;
int calib_val = 900;
int error_count =0;

//--------------------------------



void setup()
{
  pinMode(LDR, INPUT);

    Servo1.attach (Servo_pin); //Il Servo1 è collegato al pin digitale 

    pinMode(SIGNAL_TO_ESP, OUTPUT); digitalWrite(SIGNAL_TO_ESP, LOW);
    pinMode(stepMotorPin1, OUTPUT);
    pinMode(stepMotorPin2, OUTPUT);
    pinMode(stepMotorPin3, OUTPUT);
    pinMode(stepMotorPin4, OUTPUT);


    Serial.begin(9600);
    delay(2000);                        //FOR NANO BOARD ONLY!! 
    Serial.println("serial Begin!");


    /*SETUP PHOTOINTERRUPTER*/
    mytime[0] = millis();   Serial.print("mytime[0]: "); Serial.println(mytime[i]);
    i++;
}

void loop()
{
    if (start ==1) { t_0 = millis(); start = 0; cycle_stage = 1; }  //----------THIS SECTION IS EXECUTED only once


    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIME TO CURRENT ARD TIME  */
    t_current = millis();
    check_schedule();

    if (operation_over == 1){ menu(); }  //only check menu again if no operation is taking place
    else
    {
        dispense_pills();   
        success = photointerrupter();

        verify_success();
    }
    

}


void verify_success()
{

  switch (success) {
      case 0:                                //ERROR!
        operation_over = 1;
        Serial.println("ERROR when dispensing pill!!");
        break;
      case 1:                                //SUCCESS!
        operation_over = 1;
        Serial.println("Pill dispensed successfully!!");
        break;
      case 2:                                 //TRY AGAIN!   NO PILL WAS DISPENSED
        Serial.println("Reattempting pill dispensing!");
        delay(2000);
        attempts++; Serial.print("\t attempt number:"); Serial.println(attempts);
        if(attempts >= 3){ operation_over = 1; attempts = 0; }

        break;  
      default:
        Serial.println("Default!");     //-------------------------------------
  }

}

void check_schedule()
{

}

void menu()
{
    /*DEFINE MODES OF OPERATION:
  0 - insert schedule
  1 - add pills to compartment
  2 (default) - dispense pills
  +

  */
  Serial.println("\n------------------------------------------------------------");
  Serial.println("Welcome to the MENU! \n To enter the desired mode, press: ");
  Serial.println("0 - for inserting a schedule"); Serial.println("1 - for adding pills to a module storage"); Serial.println("2 - for dispensing pills");

  while (Serial.available() == 0) {} mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP

  switch (mode) {
      case 0:
        Serial.println("0 Inserted!");
          insert_schedule();  
          break;

      case 1:
        Serial.println("1 Inserted!");
          add_pills();
          break;

      default:
        operation_over = 0;  //WE START AN OPERATION, BYPASS MENU
        engine_over = 0;     //WE ALLOW DISPENSE PILLS TO OPERATE ONCE WITH THIS FLAG
        Serial.println("Start of operation!");
  }
}


void add_pills()
{

  int module_number;

  Serial.println("You are in add pills mode! \n Please insert the module number that will receive the pill!");
  while (Serial.available() == 0) {}  module_number = Serial.parseInt();
  //UNLOCKING A LOCK WOULD BE NICE HERE!

  Serial.println("Please insert the number of pills that will be added to the storage :");
  while (Serial.available() == 0) {}  number_of_pills = Serial.parseInt();

  Serial.print("Please type the medicine name that will be added to the storage number" ); Serial.print(module_number); Serial.println(":");
  while (Serial.available() == 0) {}    Serial.setTimeout(5000);   //This sets the maximum time to wait for serial data from user.
  while (Serial.available() == 0) {}    medicine = Serial.readString();


  return;
  
}

void insert_schedule()   //TO BE ALTERED! DATA OF SCHEDULE HAS TO RETURN TO THE MAIN!!!
{
  int module_number;
  

  Serial.println("You are in insert schedule mode! \n Please insert the hours '00' followed by an enter and then minutes '00' "); 
  Serial.println("insert hours, range from 0 until 23:");  while (Serial.available() == 0) {}  Schedule_time[0] =Serial.parseInt();
  Serial.println("insert minutes, range from 0 until 59"); while (Serial.available() == 0) {}  Schedule_time[1] =Serial.parseInt();
  
  Serial.println("insert module number to be operated at the specified time:"); while (Serial.available() == 0) {} module_number = Serial.parseInt();

  Serial.print("Schedule Saved!! Time:"); Serial.print(Schedule_time[0]);  Serial.print(":"); Serial.println(Schedule_time[1]);
  Serial.print("Module : "); Serial.println(module_number);

  Serial.println("Would you like to insert the pills into the storage now? \n Y for yes, N for no.");
  while (Serial.available() == 0) {
    }
    Serial.setTimeout(1000);   //This sets the maximum time to wait for serial data from user.
    String menuChoice = Serial.readString();
  
  if (menuChoice == "Y")//(!strcmp(menuChoice, ("Y"))
  {
    add_pills();  //add module number to be passed to add_pills       ------------                        TO DO
  } 
  else
  {
    return;
  }

}

int photointerrupter()
{

    delay(Delay_time_photoint);  int light = analogRead(LDR);

    if (light < calib_val)                      //detection of light beam obstruction
    {
        Serial.println(light); 
        counter++; mytime[i] = millis();          //record passing of pill and its timestamp
    
        if(mytime[i] - mytime[i-1] < Time_between_detections   &&   i != 0)   // if 2 pill counting events are less than 100ms apart, that means that 2 or more pills were dispensed at once!
        {
            Serial.println("DISPENSING ERROR! 2 or more pills dispensed!"); 

            send_email();              // SEND EMAIL!!!!!!           -------------------------------------------------------------------------------
            
            Serial.print("counter : ");           Serial.println(counter);    
            Serial.print("time difference : ") ;  Serial.println(mytime[i]-mytime[i-1]);
            Serial.print("mytime[i] : ") ;        Serial.println(mytime[i]);
            Serial.print("mytime[0] : ") ;        Serial.println(mytime[i-1]);  
            Serial.print("i = ");                 Serial.println(i);

            i=0; mytime[i] = millis();  //reset mytime  
            i++;  error_count++; 

            return 0;    //return to success variable on main
        }
        else
        {
            Serial.print("counter : ");             Serial.println(counter);    
            Serial.print("time difference : ") ;    Serial.println(mytime[i]-mytime[i-1]);
            Serial.print("mytime[i] : ") ;          Serial.println(mytime[i]);
            Serial.print("mytime[0] : ") ;          Serial.println(mytime[i-1]);
            Serial.print("i = ");                   Serial.println(i);
            i++;

        }

    }

    else if (t_current - mytime[i] > TIMEOUT_COUNTER  &&  i != 0)   //     CASE THAT NO PILL IS DISPENSED.    ----------------------------------
    {
        Serial.println("TIMEOUT FOR PILL DETECTION! \t NO PILL DISPENSED");  //TO ADD! TRY AGAIN TO DISPENSE PILL!
        Serial.print("current time - last event time > TIMEOUT = "); Serial.println(t_current - mytime[i]);
        Serial.print("t_current: ") ;          Serial.println(t_current);
        Serial.print("mytime["); Serial.print(i); Serial.print("] : ");  Serial.println(mytime[i]);

        return 2;     //return to success variable on main
    }


}

int alarm()
{
  
  /* INSERT CODE FOR ALARM SYSTEM HERE (sound buzz) */

  //WHEN schedule_time[0] == 'value'  &&  schedule_time[1]  == 'value'

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
          Serial.println("lock pill! Getting ready for Stage 2");
      }


/*                ESSENTIAL FOR PHOTOINTERRUPTER TO WORK HERE                        */

      if((t_current - t_0 >1000) &&  cycle_stage == 2)   //wait 500ms                             //   STAGE 2
      {   
          open_gate();
          t_0 = t_current;
          cycle_stage = 3;
          Serial.println("open gate! Getting ready for Stage 3");
      }
  

      if((t_current - t_0 >2000) && cycle_stage == 3)   //wait 2000ms                             //   STAGE 3
      {   
          close_gate();
          t_0 = t_current;
          cycle_stage = 4;
          Serial.println("close gate! Getting ready for Stage 4");
      }


      if((t_current - t_0 >1000) && cycle_stage == 4 )  //wait 500ms                             //   STAGE 3
      {   
          unlock_pill();
          t_0 = t_current;
          cycle_stage = 1;  engine_over = 1;                                //SETS THE END OF THE ENGINE OPERATION!!
          Serial.println("unlock pill! Getting ready for Stage 1");    
      }
  }
}


void lock_pill()   // Stepper motor operation
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

void unlock_pill()   //Stepper_motor operation
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


void send_email()
{
    digitalWrite(SIGNAL_TO_ESP, HIGH);
    Serial.println("Sending_Email_to_Caregiver!!");
    delay(30);
    digitalWrite(SIGNAL_TO_ESP, LOW);
    delay(2000);
}
