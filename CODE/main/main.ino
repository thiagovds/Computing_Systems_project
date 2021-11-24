#include <Servo.h> 


#define Time_interval_stepper 1000
#define Time_interval_servo 500
#define stepper_steps 50

#define stepMotorPin1 10
#define stepMotorPin2 11
#define stepMotorPin3 12
#define stepMotorPin4 13

#define LDR A1
#define Servo_pin 9

/*  INITIALIZATION OF GLOBAL VARIABLES FOR MAIN  ----------------------  */

Servo Servo1;

int cycle_stage;
int start = 1;

int mode, operation_over = 1;
int motor_Speed = 4;

int Schedule_time[2];
unsigned long t_current, t_0;

//------- ADD PILLS global variables
String medicine;
int number_of_pills;

int success = 0;

/*-----------------------*/

//-------------PHOTOINTERRUPTER GLOBAL VARIABLES --
int counter;
int mytime[3];
int i;
int calib_val = 900;
int error_count =0;

//--------------------------------



void setup()
{
  pinMode(LDR, INPUT);

    Servo1.attach (Servo_pin); //Il Servo1 è collegato al pin digitale 

    pinMode(stepMotorPin1, OUTPUT);
    pinMode(stepMotorPin2, OUTPUT);
    pinMode(stepMotorPin3, OUTPUT);
    pinMode(stepMotorPin4, OUTPUT);
    
    Serial.begin(9600);
    Serial.println("serial Begin!");

}

void loop()
{
  if (start ==1)                      //----------THIS SECTION IS EXECUTED only once
    {
        t_0 = millis(); start = 0;
        cycle_stage = 1;
    }                                      //------------------------------ END SECTION

    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIME TO CURRENT ARD TIME  */
    t_current = millis();

  if (operation_over == 1){ menu(); }  //only check menu again if no operation is taking place
  else{
    dispense_pills();   //erial.println("Dispensing pill!");
    success = photointerrupter();
    if (success == 1)
    {
    	operation_over == 1;
    }
    else
    {

    }
  }




}

void menu()
{
    /*DEFINE FSM: MODES
  0 - insert schedule
  1 - add pills to compartment
  2 (default) - dispense pills
  +
  +

  */

  Serial.println("Welcome to the MENU! \n To enter the desired mode, press: ");
  Serial.println("0 - for inserting a schedule"); Serial.println("1 - for adding pills to a module storage"); Serial.println("any key - for dispensing pills");

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


  	/*delay(); LETS TRY WITH NO DELAY*/  int light = analogRead(LDR);

    if (light < calib_val)                      //detection of light beam obstruction
    {
      counter++; mytime[i] = millis();          //record passing of pill and its timestamp
    
      if(mytime[i] - mytime[i-1] < 1000   &&   i != 0)   // if 2 pill counting events are less than 100ms apart, that means that 2 or more pills were dispensed at once!
      {
          Serial.println("DISPENSING ERROR! 2 or more pills dispensed!"); 

            send_email();//ADD LINE HERE TO SEND EMAIL!!!!!!           -------------------------------------------------------------------------------

          i=0; mytime[i] = millis();  //reset mytime  
          error_count++;
        
          Serial.print("counter : ");           Serial.println(counter);    
          Serial.print("time difference : ") ;  Serial.println(mytime[i]-mytime[i-1]);
          Serial.print("mytime[i] : ") ;        Serial.println(mytime[i]);
          Serial.print("mytime[0] : ") ;        Serial.println(mytime[i-1]);  
          Serial.print("i = ");                 Serial.println(i);
        
          return 0;
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

/*

  create error-success flags

*/
}

int alarm()
{
  
  /* INSERT CODE FOR ALARM SYSTEM HERE (sound buzz) */

}

void dispense_pills()
{
  if((t_current - t_0 >1000) &&  (cycle_stage == 1))                                            //   STAGE 1
    {       
        lock_pill();
        t_0 = t_current;
        cycle_stage = 2;
        Serial.println("lock pill! Getting ready for Stage 2");
    }


/*              ESSENTIAL FOR PHOTOINTERRUPTER TO WORK HERE                        */

  if((t_current - t_0 >1000) &&  cycle_stage == 2)   //wait 500ms                             //   STAGE 2
    {   
        open_gate();
        t_0 = t_current;
        cycle_stage = 3;
        Serial.println("open gate! Getting ready for Stage 3");
    }
  

/*                                                            */
    if((t_current - t_0 >2000) && cycle_stage == 3)   //wait 2000ms                             //   STAGE 3
    {   
        close_gate();
        t_0 = t_current;
        cycle_stage = 4;
        Serial.println("close gate! Getting ready for Stage 4");
    }
    //wait 500ms

    if((t_current - t_0 >1000) && cycle_stage == 4 )  //wait 500ms                             //   STAGE 3
    {   
        unlock_pill();
        t_0 = t_current;
        cycle_stage = 1;
        //engine_over
        Serial.println("unlock pill! Getting ready for Stage 1");    
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
  
}
