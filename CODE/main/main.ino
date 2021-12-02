#include <WiFiNINA.h>
#include <RTCZero.h>
#include <Servo.h>
#include <SPI.h> 


#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status


#define MAX_num_of_routines 30
#define MAX_num_of_modules 4


/* PINOUT SELECTION   --------------------------------------*/
#define LDR A1
#define SIGNAL_TO_ESP 2
#define BUZZER 3        //PWM WE CAN TUNE THE FREQUENCY!!
#define Servo_pin 9

#define stepMotorPin1 10
#define stepMotorPin2 11
#define stepMotorPin3 12
#define stepMotorPin4 13

#define SEL0 21 //BLUE CABLE
#define SEL1 20 //PURPLE CABLE -----------

/* CALIBRATION CONSTANTS FOR STEPPER   -----------------------*/
#define Time_interval_stepper 1000
#define Time_interval_servo 500
#define stepper_steps 50


/* CALIBRATION CONSTANTS FOR PHOTOINTERRUPTER -----------------*/
#define Delay_time_photoint 20
#define TIMEOUT_COUNTER 10000
#define Time_between_detections 1000          //to set around 100ms ?


/* Create an RTC (Real Time Clock) object  ------------------------------------ RTC */

RTCZero rtc;

/* Change these values to set the current initial time */

 byte seconds = 0;
 byte minutes = 25;
 byte hours = 17;

/* Change these values to set the current initial date */

 byte day = 30;
 byte month = 11;
 byte year = 21;



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
int insert = 0;
int edit = 0;
boolean buzz = 0;


unsigned long t_current, t_0;
/*         GLOBAL VARIABLES FOR verify_success FUNCTION       */
int attempts = 0;
int success = 0;


/*         ADD PILLS global variables              */
String medicine[MAX_num_of_modules + 1];
int number_of_pills[MAX_num_of_modules + 1];



/*        PHOTOINTERRUPTER GLOBAL VARIABLES        */
int counter;
int mytime[20];   //size can be changed
int i=1;
int calib_val = 900;
int error_count = 0;





void setup()
{
    setup_pins();

    Serial.begin(9600); delay(2000); Serial.println("serial Begin!\n");

    rtc.begin();
    Wifi_setup();
    real_time_clock_setup();

    rtc.attachInterrupt(alarmMatch);
    print_date_time();

}

void loop()
{

    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIMESTAMP TO CURRENT TIMESTAMP  */
    t_current = millis();
    check_schedule();

    if (operation_over == 1)
    {
        //buzz = 0; digitalWrite(BUZZER, LOW); digitalWrite(LED_BUILTIN, LOW);
        menu();
    }           //only check menu again if no operation is taking place
    else
    {
        if (buzz == 1)
        {
            //later ADD BUTTON PRESS TO STOP BUZZ AND ACTIVATE PILL!
            dispense_pills();   
            success = photointerrupter();
        }

        if (engine_over == 1) {verify_success();}
    }
    

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

void Wifi_setup()
{
    // check for the WiFi module:

    if (WiFi.status() == WL_NO_MODULE) 
    {
        Serial.println("Communication with WiFi module failed!");
        
        // don't continue
        while (true);
    }

    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION) {Serial.println("Please upgrade the firmware");}
    

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) 
    {
      Serial.print("Attempting to connect to WPA SSID: ");Serial.println(ssid);
    
      // Connect to WPA/WPA2 network:
      status = WiFi.begin(ssid, pass);    
      
      delay(2000);// wait 10 seconds for connection:

    }
    // you're connected now, so print out the data:
    Serial.print("You're connected to the network");
    printCurrentNet();   printWifiData();
    
}

void real_time_clock_setup()   //FURTHER IMPLEMENTATION ACQUIRE DATA FROM WEB RTC
{
    Serial.println("Lets configure the date and time in your RAPID! ");
    Serial.println("Please insert the day number of today: ");
    while (Serial.available() == 0) {} day = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    date_error_check(day, 31 ,0 );

    Serial.println("Please insert the current month number: ");
    while (Serial.available() == 0) {} month = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    date_error_check(month, 12, 0);

    Serial.println("Now, please insert the current year: ");
    while (Serial.available() == 0) {} year = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    date_error_check(year, 99, 20);

    Serial.println("Please insert the hours: ");
    while (Serial.available() == 0) {} hours = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    date_error_check(hours, 24, -1);

    Serial.println("And finally please insert the minutes: ");
    while (Serial.available() == 0) {} minutes = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    date_error_check(minutes, 60, -1);
    
    seconds = 0;
    
    rtc.begin(); // initialize RTC 24H format
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, year);

    //rtc.setAlarmTime(16, 0, 10);
    //rtc.enableAlarm(rtc.MATCH_HHMMSS);

}

void date_error_check(int d, int max_n, int min_n)
{
    while (d > max_n  ||  d <= min_n)  //------------INPUT ERROR CHECKING  ------------------- 
        {
            Serial.println("invalid input! \n Try again: ");
            while (Serial.available() == 0) {} 
            d = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
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
    Serial.println("Alarm Match!\t Time to take the pill!\t BUZZING!");
    digitalWrite(BUZZER, HIGH); digitalWrite(LED_BUILTIN, HIGH);
    buzz = 1;
}


int set_alarm()
{
  //WHEN schedule_time[][0] == 'value'  &&  schedule_time[1]  == 'value'
  Serial.println("Alarm set to:");
  for(int a=0; a<Y; a++)
  {
    rtc.setAlarmTime(Schedule_time[a][0], Schedule_time[a][1], 00);
    rtc.enableAlarm(rtc.MATCH_HHMMSS); //rtc.enableAlarm(rtc.MATCH_DHHMMSS);
    print2digits(Schedule_time[a][0]);  Serial.print(":");
    print2digits(Schedule_time[a][1]);  Serial.println("");
  }
  Serial.println("Alarm updated!!!");
}

void check_schedule()
{
    
}

void menu()
{

  print_date_time();

  Serial.println("\n------------------------------------------------------------");
  Serial.println("Welcome to the MAIN MENU! \n To enter the desired mode, press: ");
  Serial.println("1 - for displaying your full schedule.");
  Serial.println("2 - for editing routine schedule."); 
  Serial.println("3 - for refilling pills to a module storage."); 
  Serial.println("4 - for dispensing pills.");
  Serial.println("5 - for displaying Wifi connection settings.");  
  Serial.println("------------------------------------------------------------");

    while (Serial.available() == 0) {}  mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
    while (mode > 5  ||  mode<=0)  //------------INPUT ERROR CHECKING  ------------------- 
    {
        while (Serial.available() == 0) {} 
        mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
        Serial.println("The mode inserted mode does not exist! \n Try again: ");
    }


  switch (mode) 
  {
      case 1:
        Serial.println("\n1 Inserted!");
        print_schedule();
        break;

      case 2:
        Serial.println("\n2 Inserted!");
        edit_schedule_menu();
        break;

      case 3:
        Serial.println("\n3 Inserted!");
        refill_module();
        break;

      case 4:  //dispensing pills!  mode 4
        Serial.println("\n4 Inserted!");

        if (schedule_enumerator == 0)
        {
            Serial.println("You have no pill routines! Nothing to dispense!.");
            return;
        }

        selector_function();  //We activate the correct module to be operated!
        operation_over = 0;  //WE START AN OPERATION, BYPASS MENU
        engine_over = 0;     //WE ALLOW DISPENSE PILLS TO OPERATE ONCE WITH THIS FLAG
        Serial.println("Start of operation!");
        break;

      default:  
        printCurrentNet();
  }
}


void refill_module()   //when refill take into the account the previous amount of pills in a module!!!
{
    if (schedule_enumerator == 0 && insert == 0)
    {
      Serial.println("You have no pill routines! Modules cannot be refilled.");
      return;
    }
    else if(insert == 0)
    { 
      print_schedule();
      Serial.println("You are in add pills mode! \nPlease insert the module number that will be refilled!");
      while (Serial.available() == 0) {}  module_number[schedule_enumerator] = Serial.parseInt();
      //UNLOCKING A LOCK WOULD BE NICE HERE!
      selector_function();  //We activate the correct module to be operated!
    }
    
    Serial.print("Please insert the number of pills that will be added to module number "); Serial.print(module_number[schedule_enumerator]); Serial.println(": ");
    while (Serial.available() == 0) {}  number_of_pills[module_number[schedule_enumerator]] = Serial.parseInt();
    Serial.println(number_of_pills[module_number[schedule_enumerator]]);
    
    Serial.print("Please type the medicine name that will be added to the module number " ); Serial.print(module_number[schedule_enumerator]); Serial.println(": ");
    //while (Serial.available() == 0) {}    Serial.setTimeout(500);   //This sets the maximum time to wait for serial data from user.
    while (Serial.available() == 0) {}    medicine[module_number[schedule_enumerator]] = Serial.readString();
    Serial.println(medicine[module_number[schedule_enumerator]]);
    
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
            Serial.println("The inserted mode does not exist! \n Try again: ");
            while (Serial.available() == 0) {} 
            schedule_menu_mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
        }
        switch (schedule_menu_mode) {
            case 1:
              Serial.println("\n1 Inserted, inserting schedule!\n");
              insert_schedule_routine();
              break;
      
            case 2:
              Serial.println("\n2 Inserted, editing schedule!\n");
              edit_schedule();
              break;
      
            case 3:
              Serial.println("\n3 Inserted, delete routine!\n");
              delete_schedule_routine();
              break;
      
            default:  //exit the menu
              Serial.println("\n4 Inserted, exiting the menu!\n");
              return;
        }  
                
    }


}

void insert_schedule_routine()   //TO BE ALTERED! DATA OF SCHEDULE HAS TO RETURN TO THE MAIN!!!           
{
  if (edit == 0){ Serial.println("\nYou are in insert schedule routine mode! "); }

  insert = 1;
  Serial.println("Lets set the time of your routine..."); 
  Serial.println("Please insert the hours, range from 0 until 23:");  while (Serial.available() == 0) {}  Schedule_time[schedule_enumerator][0] =Serial.parseInt(); 
  while (Schedule_time[schedule_enumerator][0] < 0  ||  Schedule_time[schedule_enumerator][0] > 23)
    {   
        Serial.println(Schedule_time[schedule_enumerator][0]);
        Serial.println("The hour inserted does not exist! \n Try again: ");
        while (Serial.available() == 0) {}  Schedule_time[schedule_enumerator][0] =Serial.parseInt(); 
    }
  Serial.println(Schedule_time[schedule_enumerator][0]);
  
  Serial.println("Please insert the minutes, range from 0 until 59"); while (Serial.available() == 0) {}  Schedule_time[schedule_enumerator][1] =Serial.parseInt(); 
  while (Schedule_time[schedule_enumerator][1] < 0  ||  Schedule_time[schedule_enumerator][1] > 59)
    {   
        Serial.println(Schedule_time[schedule_enumerator][1]);
        Serial.println("The minutes inserted do not exist! \n Try again: ");
        while (Serial.available() == 0) {}  Schedule_time[schedule_enumerator][1] =Serial.parseInt(); 
    }
  Serial.println(Schedule_time[schedule_enumerator][1]);
  
  Serial.println("Insert module number to be operated at the specified time:"); while (Serial.available() == 0) {} module_number[schedule_enumerator] = Serial.parseInt(); 
  
  module_number_error_check();

  Serial.println(module_number[schedule_enumerator]);
  
  selector_function();  //We activate the correct module to be operated!
  Serial.print("Schedule Saved!! Time: "); Serial.print(Schedule_time[schedule_enumerator][0]);  Serial.print(":"); Serial.println(Schedule_time[schedule_enumerator][1]);
  
  Serial.print("Module : "); Serial.println(module_number[schedule_enumerator]);  

  //----------------------------------

  Serial.println("Would you like to insert the pills into the storage now? \n Y for yes, N for no.");
  while (Serial.available() == 0) {
    }
  Serial.setTimeout(500);   //This sets the maximum time to wait for serial data from user.
  String menuChoice = Serial.readString();
  
  if ((menuChoice == "Y") || (menuChoice == "y"))                                                       //ERROR CHECK!!!!
  {
    refill_module();  //add module number to be passed to refill_module       ------------                        TO DO
  }
  set_alarm();

  schedule_enumerator++;
  insert = 0;
  return;

}

void module_number_error_check()
{
  int flag;
  do{
    flag = 0;
    if(module_number[schedule_enumerator] == 0 || (module_number[schedule_enumerator] >= MAX_num_of_modules) ) { flag++; }
    else
    {
        for(int m=0; m<=schedule_enumerator; m++)
        {    
          if ((module_number[schedule_enumerator] == module_number[m]) && (m !=schedule_enumerator))
          {   
              flag++;
          }
        }
    }
    
    if (flag != 0) 
    {
        Serial.println(Schedule_time[schedule_enumerator][1]);
        Serial.println("The module number inserted is occupied or invalid! \n Try again: ");
        while (Serial.available() == 0) {}  module_number[schedule_enumerator] =Serial.parseInt(); 
    }

  }while (flag != 0);

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
    Serial.print("schedule_enumerator"); Serial.println(schedule_enumerator);
    if (edit == 0)
    {
        Serial.println("Insert routine number to delete : "); r = input_routine_number(); Serial.println(r);
    }

    for (int k=r-1; k < Y-1; k++)
    {
        for(int l =0; l < X; l++)
        {
            Schedule_time[k][l] = Schedule_time[k+1][l];   //move all values higher than the value to edit
        }

        medicine[module_number[k]] = medicine[module_number[k+1]];
        number_of_pills[module_number[k]] = number_of_pills[module_number[k+1]];
        module_number[k] = module_number [k+1];
        print_schedule();

    }
    
    Schedule_time[Y-1][0] = 0; Schedule_time[Y-1][1] = 0;  //delete that routine number
    schedule_enumerator--;

}

void print_schedule()
{
  /*         --------------------     PRINT THE WHOLE ROUTINE    -----------------------------    */
        Serial.println("This is your full routine schedule:");

        for (int k = 0; k < schedule_enumerator; k++) 
        {
            Serial.print(k+1); Serial.print(" - "); Serial.print(medicine[module_number[k]]); Serial.print("\t   Time: "); 
            for (int l =0; l < X; l++) 
            {
                Serial.print(Schedule_time[k][l]);
                if (l) Serial.print(" ");
                else Serial.print(":");
            }

            Serial.print("\t   Module number :"); Serial.print(module_number[k]); Serial.print("\t   Number of Pills: "); Serial.println(number_of_pills[module_number[k]]);
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



void printWifiData() 
{
    
    // print your board's IP address:
    
    IPAddress ip = WiFi.localIP();
    
    Serial.print("IP Address: ");
    Serial.println(ip);
    //Serial.println(ip);
    
    // print your MAC address:
    
    byte mac[6];
    
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");  printMacAddress(mac);
    Serial.println("------------------------------------------------------------\n");
}   
    
void printCurrentNet() 
{
    
    // print the SSID of the network you're attached to:
    
    Serial.print("\nSSID: ");  Serial.println(WiFi.SSID());
    
    // print the MAC address of the router you're attached to:
    
    byte bssid[6];
    WiFi.BSSID(bssid);
    
    Serial.print("BSSID: ");  printMacAddress(bssid);
    
    // print the received signal strength:
    
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");Serial.println(rssi);
    
    // print the encryption type:
    
    byte encryption = WiFi.encryptionType();
    
    Serial.print("Encryption Type:");
    Serial.print(encryption, HEX); Serial.println("");
}

void printMacAddress(byte mac[]) 
{
  for (int i = 5; i >= 0; i--) 
  {
    if (mac[i] < 16) 
    {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) 
    {
      Serial.print(":");

    }
  }
  Serial.println();
}
