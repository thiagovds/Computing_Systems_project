#include <WiFiNINA.h>
#include <RTCZero.h>
#include <Servo.h> 

#define MAX_num_of_routines 30
#define MAX_num_of_modules 4


/* PINOUT SELECTION   --------------------------------------*/
#define LDR A1
#define SIGNAL_TO_ESP 2
#define Servo_pin 9

#define SEL0 21 //BLUE CABLE
#define SEL1 20 //PURPLE CABLE -----------


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


/* Create an RTC (Real Time Clock) object  ------------------------------------ RTC */

RTCZero rtc;

/* Change these values to set the current initial time */

const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */

const byte day = 25;
const byte month = 9;
const byte year = 15;



/*  INITIALIZATION OF GLOBAL VARIABLES FOR MAIN  ----------------------  */
int mode, operation_over = 1;
//int start = 1;


/*    GLOBAL VARIABLES FOR ENGINES    */
Servo Servo1;   // create Servo object

int cycle_stage;
int motor_Speed = 4;
int engine_over = 0;

/*    GLOBAL VARIABLES FOR SCHEDULE    */

int X = 2;
int r, Y;

int Schedule_time[MAX_num_of_routines][2];
int schedule_enumerator = 0;  //variable to keep track of how many pill-taking events in a complex schedule
int module_number[MAX_num_of_routines];
int insert;
int edit = 0;


unsigned long t_current, t_0;
/*         GLOBAL VARIABLES FOR verify_success FUNCTION       */
int attempts = 0;
int success = 0;


/*         ADD PILLS global variables              */
String medicine[MAX_num_of_modules + 1];
int number_of_pills[MAX_num_of_routines];



/*        PHOTOINTERRUPTER GLOBAL VARIABLES        */
int counter;
int mytime[20];   //size can be changed
int i=1;
int calib_val = 900;
int error_count = 0;





void setup()
{
    pinMode(LDR, INPUT);
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);

    Servo1.attach (Servo_pin); //Il Servo1 è collegato al pin digitale 
    pinMode(stepMotorPin1, OUTPUT);
    pinMode(stepMotorPin2, OUTPUT);
    pinMode(stepMotorPin3, OUTPUT);
    pinMode(stepMotorPin4, OUTPUT);

    /* SETUP Send_email() */
    pinMode(SIGNAL_TO_ESP, OUTPUT); digitalWrite(SIGNAL_TO_ESP, LOW);


    Serial.begin(9600); while (!Serial); Serial.println("serial Begin!");

    rtc.begin(); // initialize RTC 24H format
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, year);

    //rtc.setAlarmTime(16, 0, 10);
    //rtc.enableAlarm(rtc.MATCH_HHMMSS);

    rtc.attachInterrupt(alarmMatch);

    print_date_time();

    /* SETUP ENGINES TIMING */
    t_0 = millis(); cycle_stage = 1;

    /*SETUP PHOTOINTERRUPTER*/
    mytime[0] = t_0;   //Serial.print("mytime[0]: "); Serial.println(mytime[i]);
    i++;
}

void loop()
{

    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIMESTAMP TO CURRENT TIMESTAMP  */
    t_current = millis();
    check_schedule();

    if (operation_over == 1){ menu(); }  //only check menu again if no operation is taking place
    else
    {

        dispense_pills();   
        success = photointerrupter();

        if (engine_over == 1) {verify_success();}
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
        if(attempts > 3){ operation_over = 1; attempts = 0; }

        break;  
      default:
        Serial.println("Default!");     //-------------------------------------
  }

}


void alarmMatch()
{
    Serial.println("Alarm Match!");
}

int alarm()
{
  
  /* INSERT CODE FOR ALARM SYSTEM HERE (sound buzz) */

  //WHEN schedule_time[][0] == 'value'  &&  schedule_time[1]  == 'value'
  for(int a=0; a<Y; a++)
  {
    rtc.setAlarmTime(Schedule_time[a][0], Schedule_time[a][0], 00);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
  }
}

void check_schedule()
{
    
}

void menu()
{
    /*DEFINE MODES OF OPERATION:
  0 - insert schedule
  1 - edit schedule
  2 - add pills to compartment
  3 - dispense pills
  +

  */
  Serial.println("\n------------------------------------------------------------");
  Serial.println("Welcome to the MAIN MENU! \n To enter the desired mode, press: ");
  Serial.println("1 - for displaying your full schedule");
  Serial.println("2 - for editing routine schedule"); 
  Serial.println("3 - for refilling pills to a module storage"); Serial.println("4 - for dispensing pills");
  Serial.println("------------------------------------------------------------");

    while (Serial.available() == 0) {}  mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    while (mode > 3  ||  mode<=0)  //------------INPUT ERROR CHECKING  ------------------- 
    {
        while (Serial.available() == 0) {} 
        mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
        Serial.println("The mode inserted mode does not exist! \n Try again: ");
    }


  switch (mode) 
  {
      case 1:
        Serial.println("1 Inserted!");
        edit_schedule_menu();
        break;

      case 2:
        Serial.println("2 Inserted!");
        refill_module();
        break;

      case 3:
        Serial.println("3 Inserted!");
        print_schedule();
        break;

      default:  //dispensing pills!  mode 3
        selector_function();  //We activate the correct module to be operated!
        operation_over = 0;  //WE START AN OPERATION, BYPASS MENU
        engine_over = 0;     //WE ALLOW DISPENSE PILLS TO OPERATE ONCE WITH THIS FLAG
        Serial.println("Start of operation!");
  }
}


void refill_module()   //when refill take into the account the previous amount of pills in a module!!!
{
  Serial.print("insert variable :"); Serial.println(insert);
  if(insert = 0)
  { 
    Serial.println("You are in add pills mode! \n Please insert the module number that will receive the pill!");
    while (Serial.available() == 0) {}  module_number[schedule_enumerator] = Serial.parseInt();
    //UNLOCKING A LOCK WOULD BE NICE HERE!
    selector_function();  //We activate the correct module to be operated!
  }

  Serial.println("Please insert the number of pills that will be added to the storage :");
  while (Serial.available() == 0) {}  number_of_pills[schedule_enumerator] = Serial.parseInt();

  Serial.print("Please type the medicine name that will be added to the storage number:  " ); Serial.print(module_number[schedule_enumerator]); Serial.println("");
  while (Serial.available() == 0) {}    Serial.setTimeout(5000);   //This sets the maximum time to wait for serial data from user.
  while (Serial.available() == 0) {}    medicine[module_number[schedule_enumerator]] = Serial.readString();
  
}

void edit_schedule_menu()        //////////////////////////////////////////////////////
{
    Y = 1 + schedule_enumerator; Serial.print("schedule_enumerator = "); Serial.println(schedule_enumerator);
    
    int schedule_menu_mode = 0;


    if (schedule_enumerator == 0){insert_schedule_routine(); return;}
    else
    {
        Serial.println("\n------------------------------------------------------------");
        Serial.print("You are in the EDIT SCHEDULE MENU. ");
        Serial.println("\n------------------------------------------------------------");

      print_schedule();

        Serial.println("To enter the desired mode, press:\n \t ----------------");
        Serial.println("1 - for inserting a routine."); 
        Serial.println("2 - for editing a routine in the schedule."); 
        Serial.println("3 - for deleting a routine from the schedule.");
        Serial.println("4 - to exit!");
      
        while (Serial.available() == 0) {} schedule_menu_mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
        while (schedule_menu_mode > 4  ||  schedule_menu_mode <= 0)  //------------INPUT ERROR CHECKING  ------------------- 
        {
            while (Serial.available() == 0) {} 
            schedule_menu_mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
            Serial.println("The inserted mode does not exist! \n Try again: ");
        }
        switch (schedule_menu_mode) {
            case 1:
              Serial.println("1 Inserted, inserting schedule!\n");
              insert_schedule_routine();
              break;
      
            case 2:
              Serial.println("2 Inserted, editing schedule!\n");
              edit_schedule();
              break;
      
            case 3:
              Serial.println("3 Inserted, delete routine!\n");
              delete_schedule_routine();
              break;
      
            default:  //exit the menu
              Serial.println("4 Inserted, exiting the menu!\n");
              return;
        }  
                
    }


}

void insert_schedule_routine()   //TO BE ALTERED! DATA OF SCHEDULE HAS TO RETURN TO THE MAIN!!!           
{
  if (edit == 0){ Serial.println("You are in insert schedule routine mode! "); }

  insert = 1;
  Serial.println("Please insert the hours '00' followed by an enter and then minutes '00' "); 
  Serial.println("insert hours, range from 0 until 23:");  while (Serial.available() == 0) {}  Schedule_time[schedule_enumerator][0] =Serial.parseInt();
  Serial.println("insert minutes, range from 0 until 59"); while (Serial.available() == 0) {}  Schedule_time[schedule_enumerator][1] =Serial.parseInt();
  
  Serial.println("insert module number to be operated at the specified time:"); while (Serial.available() == 0) {} module_number[schedule_enumerator] = Serial.parseInt();

  selector_function();  //We activate the correct module to be operated!
  Serial.print("Schedule Saved!! Time: "); Serial.print(Schedule_time[schedule_enumerator][0]);  Serial.print(":"); Serial.println(Schedule_time[schedule_enumerator][1]);
  
  Serial.print("Module : "); Serial.println(module_number[schedule_enumerator]);  //PRINT THE NUMBER OF MODULES OCCUPIED, SHOW WHICH MODULES ARE EMPTY\\OCCUPIED AND WITH WHAT!


  //----------------------------------

  Serial.println("Would you like to insert the pills into the storage now? \n Y for yes, N for no.");
  while (Serial.available() == 0) {
    }
  Serial.setTimeout(7000);   //This sets the maximum time to wait for serial data from user.
  String menuChoice = Serial.readString();
  
  if ((menuChoice == "Y") || (menuChoice == "y"))                                                       //ERROR CHECK!!!!
  {
    refill_module();  //add module number to be passed to refill_module       ------------                        TO DO
  }

  schedule_enumerator++;
  insert = 0;
  return;

}

void edit_schedule()
{
    Y = 1 + schedule_enumerator;
    int temp = schedule_enumerator;

    edit = 1;

    Serial.println("Insert routine number to be edited : "); 
    r = input_routine_number(); schedule_enumerator = r-1;

    delete_schedule_routine();
    insert_schedule_routine();

    edit = 0; schedule_enumerator = temp;
}

int input_routine_number()
{
    while (Serial.available() == 0) {}  r = Serial.parseInt();   //CHECKING FOR CORRECT INPUT FROM USER
    while (r >= Y  ||  r<=0)
    {
        Serial.println("That routine number does not exist! \n Try again: ");
        while (Serial.available() == 0) {}  r = Serial.parseInt();
    }
    return r;
}

void delete_schedule_routine()  //enumerator fix
{ 
    Y = 1 + schedule_enumerator; 
    
    //add bypass if editing
    Serial.println("Insert routine number to delete : "); 
    r = input_routine_number();

    for (int k=r-1; k < Y; k++)
    {
        for(int l =0; l < X; l++)
        {
            Schedule_time[k][l] = Schedule_time[k+1][l];   //move all values higher than the value to edit 
        }
    }
    
    Schedule_time[r-1][0] = 0; Schedule_time[r-1][1] = 0;  //delete that routine number
    schedule_enumerator--;

}

void print_schedule()
{
  /*         --------------------     PRINT THE WHOLE ROUTINE    -----------------------------    */
        Serial.println("This is your full routine schedule:");

        for (int k = 0; k < schedule_enumerator; k++) 
        {
            Serial.print(k+1); Serial.print(" - "); Serial.print(medicine[module_number[k]]); Serial.print("\t   Time : "); 
            for (int l =0; l < X; l++) 
            {
                Serial.print(Schedule_time[k][l]);Serial.print("  ");
            }

            Serial.println("");
        }
        Serial.println("\n------------------------------------------------------------");
/*                  ---------------------------------------------------------                  */
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



void selector_function()
{
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
    delay(31);
    digitalWrite(SIGNAL_TO_ESP, LOW);
    delay(2000);
}

void print2digits(int number) 
{
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}


void print_date_time()
{

  // Print date...

  print2digits(rtc.getDay());  Serial.print("/");
  print2digits(rtc.getMonth());  Serial.print("/");
  print2digits(rtc.getYear());  Serial.print(" ");

  // ...and time

  print2digits(rtc.getHours());  Serial.print(":");
  print2digits(rtc.getMinutes());  Serial.print(":");
  print2digits(rtc.getSeconds());  Serial.println();

  //delay(1000);
}
