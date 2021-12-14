#include <WiFiNINA.h>
#include <RTCZero.h>
#include <Servo.h>
#include <SPI.h> 
#include <WiFiUdp.h>
// Include EEPROM-like API for FlashStorage
#include <FlashAsEEPROM.h>

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
    #define BUTTON 9

    #define Servo_pin 8
    
    #define stepMotorPin1 19//9
    #define stepMotorPin2 18//10
    #define stepMotorPin3 17//11
    #define stepMotorPin4 16//12
    
    #define SEL0 21 //BLUE CABLE
    #define SEL1 20 //PURPLE CABLE -----------
    
    /* CALIBRATION CONSTANTS FOR STEPPER   -----------------------*/
    #define Time_interval_stepper 1000
    #define Time_interval_servo 500
    #define stepper_steps 20
    
    
    /* CALIBRATION CONSTANTS FOR PHOTOINTERRUPTER -----------------*/
    #define PHOTOVOLTAGE_threshold 900
    #define Delay_time_photoint_Microseconds 25000
    #define TIMEOUT_COUNTER 1500
    #define Time_between_detections 100          //to set around 100ms ?


/* Create an RTC (Real Time Clock) object  ------------------------------------ RTC --------------------------------*/

RTCZero rtc;

/* Change these values to set the current initial time */

 byte seconds = 0;
 byte minutes = 25;
 byte hours = 17;

/* Change these values to set the current initial date */

 byte day = 30;
 byte month = 11;
 byte year = 21;

//  WIFI GLOBAL VARIABLES  ------------------------------------------------------------------------
    unsigned int localPort = 2390;      // local port to listen for UDP packets

    IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

    const int NTP_PACKET_SIZE = 48; // NTP timestamp is in the first 48 bytes of the message

    byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

    // A UDP instance to let us send and receive packets over UDP
    WiFiUDP Udp;


//--NTP------------EPOCH TO DATE-TIME CONVERTER GLOBAL VARIABLES----------------------------------------------------------------------
    static unsigned char month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    static unsigned char week_days[7] = {4,5,6,0,1,2,3};
    //Thu=4, Fri=5, Sat=6, Sun=0, Mon=1, Tue=2, Wed=3
    
    unsigned char
    ntp_hour, ntp_minute, ntp_second, ntp_week_day, ntp_day, ntp_month, leap_days, leap_year_ind ;
    
    unsigned short temp_days;
    
    unsigned 
    ntp_year, days_since_epoch, day_of_year; //epoch, 
    unsigned long epoch;
    //char key;


/*  INITIALIZATION OF GLOBAL VARIABLES FOR MAIN  ----------------------  */
    int mode, operation_over = 1;
    
    int weekdayselected[MAX_num_of_routines];
    int week_day_int;

    /*    GLOBAL VARIABLES FOR ENGINES    */
    Servo Servo1;   // create Servo object
    
    int cycle_stage;
    int motor_Speed = 4;
    int engine_over = 0;
    
/*    GLOBAL VARIABLES FOR SCHEDULE & ALARM    */

    int X = 2;
    int r, Y;
    
    int Schedule_time[MAX_num_of_routines][2]; // for hours and seconds
    int Schedule_date[MAX_num_of_routines][3]; // for day, month and year
    
    int schedule_enumerator = 0;  //variable to keep track of how many pill-taking events in a complex schedule
    int module_number[MAX_num_of_routines];
    
    boolean insert = 0;
    boolean edit = 0;
    boolean menu_var = 0;
    boolean everyday_routine[MAX_num_of_routines];
    boolean Alarm_Matched = 0;

    int alarm_played = 0;
    int ordered_index[MAX_num_of_routines];
    int long schedules_in_minutes[MAX_num_of_routines][2];
    int Alarm_day,  Alarm_month;
    int Alarm_year, Alarm_hour, Alarm_minutes;
    
    
    unsigned long t_current, t_0, t_photo_start;

/*         GLOBAL VARIABLES FOR verify_success FUNCTION       */
    int success ;


/*         ADD PILLS global variables              */
    String medicine[MAX_num_of_modules + 1];
    int number_of_pills[MAX_num_of_modules + 1];



/*        PHOTOINTERRUPTER GLOBAL VARIABLES        */
    int counter;
    int mytime[5];   //size can be changed
    int i=1;
    int calib_val = 900;
    int error_count = 0;
    int check_time_elapsed_success =0;
    int result = 33;
    boolean photo_over = 0;




void setup()
{
    setup_pins();

    Serial.begin(9600); delay(2000); Serial.println("serial Begin!\n");

    rtc.begin();
    Wifi_setup();
    
    Serial.println("\nStarting connection to server...");
    Udp.begin(localPort);
    real_time_clock_setup();

    rtc.attachInterrupt(alarmMatch);

}

void loop()
{

    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIMESTAMP TO CURRENT TIMESTAMP  */
    t_current = millis();

    update_rtc();

    if (Alarm_Matched ==1 )
    {
        Serial.println("Alarm Match!\t Time to take the pill!\t BUZZING!\n\n"); //Please press the button to dispense the pill.\n");
        digitalWrite(BUZZER, HIGH); digitalWrite(LED_BUILTIN, HIGH);

        //while(!digitalRead(BUTTON)){      }
        //Serial.println("DISPENSING... \n");

        selector_function_dispense(module_number[ordered_index[alarm_played]]);  //We activate the correct module to be operated!
        operation_over = 0;  //WE START AN OPERATION, BYPASS MENU
        engine_over = 0;     //WE ALLOW DISPENSE PILLS TO OPERATE ONCE WITH THIS FLAG
        t_photo_start = millis();
        Serial.println("Start of operation!");

        alarm_played++;
        set_alarm(alarm_played);  //alarm_played);
        Alarm_Matched = 0;

    }

    if (operation_over == 1)
    {
        digitalWrite(BUZZER, LOW); digitalWrite(LED_BUILTIN, LOW);
        menu();
    }           //only check menu again if no operation is taking place
    else
    {
        //later ADD BUTTON PRESS TO STOP BUZZ AND ACTIVATE PILL!
        dispense_pills();
        if (photo_over == 0 && (cycle_stage == 2 || cycle_stage == 3))
        {   
            success = photointerrupter();
            Serial.println(success);
        }

        if (engine_over == 1) {verify_success(); menu_var = 0; photo_over = 0;}
    }
    

}

void setup_pins()
{
    pinMode(LDR, INPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(BUTTON, INPUT);
    pinMode(SEL0, OUTPUT);
    pinMode(SEL1, OUTPUT);    

    Servo1.attach (Servo_pin); //Il Servo1 è collegato al pin digitale 
    Servo1.write(65);

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
    //i++;
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

void real_time_clock_setup()
{
    sendNTPpacket(timeServer); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);
    if (Udp.parsePacket()) 
    {
        //Serial.println("packet received");
        // We've received a packet, read the data from it
        Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    
        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, extract the two words:
    
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        //Serial.print("Seconds since Jan 1 1900 = ");
        //Serial.println(secsSince1900);
    
        // now convert NTP time into everyday time:
        //Serial.print("Unix time = ");
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        /*unsigned long*/ epoch = secsSince1900 - seventyYears; 
        // print Unix time UTC 0
        //Serial.println(epoch);
    
        conversion_epoch_to_Time_Date();
    }
        //delay(1000);    
    
}


void verify_success()
{
    error_count = 0; check_time_elapsed_success = 0;

  switch (success) {
      case 0:                                //ERROR!
        operation_over = 1;
        Serial.println("ERROR when dispensing pill!!"); delay(1000);
        send_email();

        break;
      case 1:                                //SUCCESS!
        operation_over = 1;
        Serial.println("Pill dispensed successfully!!"); delay(1000);
        break;
      case 2:                                 //TRY AGAIN!   NO PILL WAS DISPENSED
        Serial.println("Pill stuck on chamber!");
        delay(1000);
        operation_over = 1; 
        break;

      default:
        Serial.println("Default!");     //-------------------------------------
        operation_over = 1;
        Serial.print("success = "); Serial.println(success); 
        break;
  }

}


void sorting_date_time(int addend)
{
    int long count_days_of_year[MAX_num_of_routines];
    int sch = 0;

    //31,536,000 seconds,   525,600 minutes  in a 365 days year
    // month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};


  // CONVERTS DATE AND TIME INTO MINUTES from 1st second of 1st day of 2000 AND SAVES THE INDEX OF EACH INSIDE THE COLUMN 1
  
  for (sch = 0; sch < schedule_enumerator + addend; sch++)
  {

    count_days_of_year[sch] = 0;

    schedules_in_minutes[sch][0] = (long)(60*Schedule_time[sch][0] + Schedule_time[sch][1]);
    

    for(int f=0; f < Schedule_date[sch][1] - 1; f++)
    {   
        count_days_of_year[sch]+= month_days[f];
        //Serial.print("f = ");Serial.print(f); Serial.print("  ;"); 
    }
    delay(2);
    schedules_in_minutes[sch][0] += (long)( ((long)Schedule_date[sch][0] + (long)count_days_of_year[schedule_enumerator] + (365)*((long)Schedule_date[sch][2]-20))*24*60);
    Serial.print("count_days_of_year = ");Serial.println(count_days_of_year[sch]);  

    Serial.print("Schedule_date["); Serial.print(sch);  Serial.print("][0] = "); Serial.println(Schedule_date[sch][0]);
    Serial.print("Schedule in minutes unordered!["); Serial.print(sch);  Serial.print("][0] = "); Serial.println(schedules_in_minutes[sch][0]);
    

    schedules_in_minutes[sch][1] = sch;  // save index
  }


    InsertionSort(schedules_in_minutes , schedule_enumerator +addend);
    

    for(int f=0; f <= schedule_enumerator; f++)
    {          
        Serial.print("Schedule in minutes ordered!["); Serial.print(f);  Serial.print("][0] = "); Serial.println(schedules_in_minutes[f][0]);
        Serial.print("Ordered_index : "); Serial.println(ordered_index[f]);      
    }


}

void InsertionSort (long int A[][2], int n) 
{
    int i1, j1, x0, x01;

    for (i1=1; i1<n; i1++) 
    {
        x0 = A[i1][0];
        x01= A[i1][1];  //sorts also the i1ndex 

        j1 = i1 - 1;

        while (j1>=0 && x0<A[j1][0]) 
        {
            A[j1+1][0] = A[j1][0]; 
            A[j1+1][1] = A[j1][1];
            j1--;
        }
        A[j1+1][0] = x0;
        A[j1+1][1] = x01;
    }

    for(int k1=0; k1<n; k1++)
    {
        ordered_index[k1] = A[k1][1];
    }
    return;
}

void alarmMatch()
{
    
    Alarm_Matched = 1;
    //Serial.println("Alarm Match!\t Time to take the pill!\t BUZZING!");
    //Serial.write(10);

}


void set_alarm(int addend)
{
    Serial.println("setting alarm!! \n");
    Alarm_day   =   Schedule_date[ ordered_index[addend] ] [0];
    Alarm_month =   Schedule_date[ ordered_index[addend] ] [1];
    Alarm_year  =   Schedule_date[ ordered_index[addend] ] [2];
    Alarm_hour  =   Schedule_time[ ordered_index[addend] ] [0];
    Alarm_minutes = Schedule_time[ ordered_index[addend] ] [1];

    Serial.print("Alarm set to:\n"); //Serial.print(Alarm_hour);Serial.print(" : "); Serial.println(Alarm_minutes);

    rtc.setAlarmTime(Alarm_hour, Alarm_minutes, 00);

    if(!everyday_routine[ordered_index[addend]])
    {
        rtc.setAlarmDate(Alarm_day, Alarm_month, Alarm_year);
        rtc.enableAlarm(rtc.MATCH_YYMMDDHHMMSS); 
    }
    else
    {
        rtc.setAlarmDate(Alarm_day, Alarm_month, Alarm_year);
        rtc.enableAlarm(rtc.MATCH_YYMMDDHHMMSS);
    }

    for(int ind=0; ind < schedule_enumerator; ind++)
    {     
        print2digits(Schedule_time[ordered_index[ind]] [0]);  Serial.print(":");
        print2digits(Schedule_time[ordered_index[ind]] [1]);  Serial.print(", ");

        if(1)//!everyday_routine[ordered_index[addend]])
        {
            print2digits(Schedule_date[ordered_index[ind]] [0]);  Serial.print("/");
            print2digits(Schedule_date[ordered_index[ind]] [1]);  Serial.print("/");
            print2digits(Schedule_date[ordered_index[ind]] [2]);  Serial.println(" ");
        }
        else
        {
            Serial.println(" Everyday ");
        }
    }

  Serial.println("Alarm updated!!!\n");
}

void update_rtc()   //check_schedule()
{
    if (rtc.getMinutes() == 12 && (rtc.getSeconds() == 00) )  //updates rtc every hour at minutes 12 
    { 
        real_time_clock_setup();
        define_Schedule_date(0);
        sorting_date_time(0); 
    }  
 /*    for (int z=0; z<schedule_enumerator; z++)
        if (ntp_week_day == weekdayselected[z])
        {

            p =+  Schedule_date[z][0] == rtc.getDay();
            p =+  Schedule_date[z][1] == rtc.getMonth(); 
            p =+  Schedule_date[z][2] == rtc.getYear();
            p =+  Schedule_time[z][0] == rtc.getHours();
            p =+  Schedule_time[z][1] == rtc.getMinutes();

            if (p == 5) {Alarm_Matched = 1}
*/     
}

void menu()
{
    if (!menu_var)
    {
        print_date_time();
        
        Serial.println("\n------------------------------------------------------------");
        Serial.println("Welcome to the MAIN MENU! \n To enter the desired mode, press: ");
        Serial.println("1 - for displaying your full schedule.");
        Serial.println("2 - for editing routine schedule."); 
        Serial.println("3 - for refilling pills to a module storage."); 
        Serial.println("4 - for dispensing pills.");
        Serial.println("5 - for displaying Wifi connection settings and updating RTC.");  
        Serial.println("------------------------------------------------------------");
        menu_var = 1;
    }
    if (Serial.available() > 0) 
    {
        mode = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
      
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
            menu_var = 0;
            if (schedule_enumerator == 0)
            {
                Serial.println("You have no pill routines! Nothing to display!.\n\n");
                return;
            }
            else if(everyday_routine[schedule_enumerator-1])
            {
                Serial.print("Schedule Saved!! Time:\n"); //Serial.print(Schedule_time[schedule_enumerator][0]);  Serial.print(":"); Serial.println(Schedule_time[schedule_enumerator][1]);
                print_schedule_date_time(0);
            }
            else 
            {
                Serial.println("Schedule Saved!! Date and Time:\n ");
                print_schedule_date_time(0);
            }
    
            break;
    
          case 2:
            //Serial.println("\n2 Inserted!");
            edit_schedule_menu();
            menu_var = 0;   
            break;
    
          case 3:
            Serial.println("\n3 Inserted!");
            refill_module();
            menu_var = 0;
            break;
    
          case 4:  //dispensing pills!  mode 4
            Serial.println("\n4 Inserted!");
            menu_var = 0;   
            /*if (schedule_enumerator == 0)
            {
                Serial.println("You have no pill routines! Nothing to dispense!.\n\n");
                return;
            }*/
    
            selector_function_dispense(1);  //We activate the module 1 to be operated in the debug!
            operation_over = 0;  //WE START AN OPERATION, BYPASS MENU
            engine_over = 0;     //WE ALLOW DISPENSE PILLS TO OPERATE ONCE WITH THIS FLAG
            t_photo_start = millis();
            Serial.println("Start of operation!");
            break;
    
          case 5:  //print net configuration mode 5
            real_time_clock_setup();
            define_Schedule_date(0);
            sorting_date_time(0); 
            printCurrentNet();
            menu_var = 0;   
            break;
    
          default:
            menu_var = 0;
            return;
        }

    }        
}


void refill_module()   //when refill take into the account the previous amount of pills in a module!!!
{
    if (schedule_enumerator == 0 && insert == 0)
    {
      Serial.println("You have no pill routines! Modules cannot be refilled.\n\n");
      return;
    }
    else if(insert == 0)
    { 
      print_schedule_date_time(0);
      Serial.println("You are in add pills mode! \nPlease insert the module number that will be refilled!");
      while (Serial.available() == 0) {}  module_number[schedule_enumerator] = Serial.parseInt();
      module_number_error_check();
      //UNLOCKING A LOCK WOULD BE NICE HERE!
      selector_function_input();  //We activate the correct module to be operated!
    }

    Serial.print("Please insert the number of pills that will be added to module number "); Serial.print(module_number[schedule_enumerator]); Serial.println(": ");
    while (Serial.available() == 0) {}  number_of_pills[module_number[schedule_enumerator]] = Serial.parseInt();
    Serial.println(number_of_pills[module_number[schedule_enumerator]]);
    if(insert != 0){ module_number_error_check();}
    //UNLOCKING A LOCK WOULD BE NICE HERE!
    
    Serial.print("Please type the medicine name that will be added to the module number " ); Serial.print(module_number[schedule_enumerator]); Serial.println(": ");
    //while (Serial.available() == 0) {}    Serial.setTimeout(500);   //This sets the maximum time to wait for serial data from user.
    while (Serial.available() == 0) {}    medicine[module_number[schedule_enumerator]] = Serial.readString();
    Serial.println(medicine[module_number[schedule_enumerator]]);Serial.println("");
    
}

void edit_schedule_menu()        //////////////////////////////////////////////////////
{
    Y = 1 + schedule_enumerator; //Serial.print("schedule_enumerator = "); Serial.println(schedule_enumerator);
    
    int schedule_menu_mode = 0;


    if (schedule_enumerator == 0)
    {
        insert_schedule_routine();
        define_Schedule_date(0);    delay(10);   sorting_date_time(0);

        Serial.println("Schedule Saved!! Date and Time:\n ");
        print_schedule_date_time(0);

        alarm_played=0;
        set_alarm(alarm_played);
 
        return;
    }
    else
    {
        Serial.println("\n------------------------------------------------------------");
        Serial.print("You are in the EDIT SCHEDULE MENU. ");
        Serial.println("\n------------------------------------------------------------");

      print_schedule_date_time(0);

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
                define_Schedule_date(0);   delay(10);   sorting_date_time(0);

                Serial.println("Schedule Saved!! Date and Time:\n ");
                print_schedule_date_time(0);

                alarm_played=0;
                set_alarm(alarm_played);
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


void input_number_error_check(int d, int max_n, int min_n)
{
    while (d > max_n  ||  d <= min_n)  //------------INPUT ERROR CHECKING  ------------------- 
        {
            Serial.println("invalid input! \n Try again: ");
            while (Serial.available() == 0) {} 
            d = Serial.parseInt();  //get input from user in serial for mode, to be upgraded to a web service with ESP
        }
        //return d;
}

void print_weekday_options()
{   
    Serial.println("Please insert the weekday you must take your pill: ");

     Serial.print("1 - for every Sunday\n");
     Serial.print("2 - for every Monday\n");
     Serial.print("3 - for every Tuesday\n");
     Serial.print("4 - for every Wednesday\n");
     Serial.print("5 - for every Thursday\n");
     Serial.print("6 - for every Friday\n");
     Serial.print("7 - for every Saturday\n");
     Serial.println("8 - for everyday\n");


}



void insert_schedule_routine()             
{
  if (edit == 0){ Serial.println("\nYou are in insert schedule routine mode! "); }
  insert = 1;

  Serial.println("Lets set the days and time of your pill intake routine...");  
  define_Schedule_time();
  
  Serial.println("Insert module number to be operated at the specified time:"); 
  while (Serial.available() == 0) {} module_number[schedule_enumerator] = Serial.parseInt();   
  
  module_number_error_check();
  
  Serial.println(module_number[schedule_enumerator]);

  //------------------------------------------

  print_weekday_options();

  while (Serial.available() == 0) {} int routine_options = Serial.parseInt();  //get input from user in serial for mode
  input_number_error_check(routine_options, 8, 0);  
  Serial.print(routine_options);Serial.println(" Inserted!");

  weekdayselected[schedule_enumerator] = routine_options -1;

  if(weekdayselected[schedule_enumerator] > 6)
  { 
    Serial.println(" everyday routine selected!\n");
    everyday_routine[schedule_enumerator] = 1;
  }  
  else // everyday_routine[schedule_enumerator] == 0 TRUE
  { 
    everyday_routine[schedule_enumerator] = 0;
    Serial.println(everyday_routine[schedule_enumerator]);  
  }

  //-------------------------------------------------  

  selector_function_input();  //We activate the correct module to be operated!

  Serial.print("Module : "); Serial.println(module_number[schedule_enumerator]);  

  //----------------------------------

  Serial.println("Would you like to insert the pills into the storage now? \n Y for yes, N for no:  ");
  while (Serial.available() == 0) {}  Serial.setTimeout(500);   //This sets the maximum time to wait for serial data from user.
  String menuChoice = Serial.readString();
  Serial.print(menuChoice);    Serial.println("\n------------------------------------------------------------");
  if ((menuChoice == "Y") || (menuChoice == "y"))                                                       //ERROR CHECK!!!!
  {
    refill_module();  //add module number to be passed to refill_module       ------------                        TO DO
  }
  else
  {
    medicine[module_number[schedule_enumerator]] = "empty";
    number_of_pills[module_number[schedule_enumerator]] = 0;
  }


  schedule_enumerator++;
  insert = 0;
  return;

}

void module_number_error_check()
{
  int flag;
  do{
    flag = 0;
    if(module_number[schedule_enumerator] == 0 || (module_number[schedule_enumerator] > MAX_num_of_modules+1) ) { flag++; }
    else
    {
        for(int m=0; m<=schedule_enumerator; m++)
        {    
          if (medicine[module_number[schedule_enumerator]] != "empty" &&(((module_number[schedule_enumerator] == module_number[m]) && (m !=schedule_enumerator)) && number_of_pills[module_number[schedule_enumerator]] != 0))
          {   
              Serial.print("This Module number: "); Serial.print(module_number[schedule_enumerator]); 
              Serial.print(" is occupied with: ");  Serial.println(medicine[module_number[schedule_enumerator]]);
              Serial.println("Are you sure to select this module?\n  'Y' for YES or 'N' for NO ");
              while (Serial.available() == 0) {}  Serial.setTimeout(500);   //This sets the maximum time to wait for serial data from user.
              String menuChoice = Serial.readString();
              Serial.print(menuChoice);    Serial.println("\n------------------------------------------------------------");
              if ((menuChoice == "Y") || (menuChoice == "y"))                                                       //ERROR CHECK!!!!
              {
                return;  //add module number to be passed to refill_module       ------------                        TO DO
              }
              else flag++;

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

void define_Schedule_time()
{
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
}


void define_Schedule_date(int addend)
{
    week_day_int = (int)ntp_week_day;
    for(int R=0; R<schedule_enumerator + addend; R++)
    {
        if(everyday_routine[R] == 1)  //in case of everyday routine
        {
            // CASE FOR EVERYDAY ROUTINES   weekdayselected[R] == 7 
            
                if(Schedule_time[R][0] <= rtc.getHours() && (Schedule_time[R][1] <= rtc.getMinutes()) ) // case in which alarm on the same day passed by hours or minutes
                {
                    Schedule_date[R][0] = rtc.getDay() + 1;
                    Schedule_date[R][1] = rtc.getMonth(); 
                    Schedule_date[R][2] = rtc.getYear();
                }
                else
                {
                    Schedule_date[R][0] = rtc.getDay();
                    Schedule_date[R][1] = rtc.getMonth(); 
                    Schedule_date[R][2] = rtc.getYear();
                }
            continue;        
        }     
     
        else if (week_day_int < weekdayselected[R])
        {
            int SUM = rtc.getDay() + (weekdayselected[R] - week_day_int);
            if (SUM > month_days[rtc.getMonth()])  //OVERFLOW DAYS // ADD A MONTH
            {
                Schedule_date[R][0] = SUM - month_days[rtc.getMonth()];
                int month_selected = rtc.getMonth() + 1;
                if(month_selected > 12) // OVERFLOW MONTHS // ADD A YEAR
                {
                    Schedule_date[R][1] = 1;  //start from January
                    Schedule_date[R][2] = rtc.getYear() + 1;
                }
                else
                {
                    Schedule_date[R][1] = rtc.getMonth();
                    Schedule_date[R][2] = rtc.getYear();
                }
            }
            else
            {
                Schedule_date[R][0] = SUM;
                Schedule_date[R][1] = rtc.getMonth();
                Schedule_date[R][2] = rtc.getYear();
            }
        }
        else if (week_day_int > weekdayselected[R] && week_day_int < 7)
        {
            int SUM = rtc.getDay() + 7 - (week_day_int - weekdayselected[R]);
            if (SUM > month_days[rtc.getMonth()])  //OVERFLOW DAYS // ADD A MONTH
            {
                Schedule_date[R][0] = SUM - month_days[rtc.getMonth()];
                int month_selected = rtc.getMonth() + 1;
                if(month_selected > 12) // OVERFLOW MONTHS // ADD A YEAR
                {
                    Schedule_date[R][1] = 1;  //start from January
                    Schedule_date[R][2] = rtc.getYear() + 1;
                }
                else
                {
                    Schedule_date[R][1] = rtc.getMonth();
                    Schedule_date[R][2] = rtc.getYear();
                }
            }
            else
            {
                Schedule_date[R][0] = SUM;
                Schedule_date[R][1] = rtc.getMonth();
                Schedule_date[R][2] = rtc.getYear();
            }
        }
        else if(week_day_int == weekdayselected[R] && (weekdayselected[R] != 7))  //week_day_int = weekdayselected[R] OR any week_day_int
        {
             if(Schedule_time[R][0] <= rtc.getHours() && (Schedule_time[R][1] < rtc.getMinutes()) ) // case in which alarm on the same day passed by hours or minutes
             {
                 Schedule_date[R][0] = rtc.getDay() + 7;
                 Schedule_date[R][1] = rtc.getMonth(); 
                 Schedule_date[R][2] = rtc.getYear();
             }
             else
             {
                 Schedule_date[R][0] = rtc.getDay();
                 Schedule_date[R][1] = rtc.getMonth(); 
                 Schedule_date[R][2] = rtc.getYear();
             }
        }
        
    }

    return;
}




void edit_schedule()
{
    Y = 1 + schedule_enumerator;
    int temp = schedule_enumerator;

    edit = 1;

    Serial.println("Insert routine number to be edited : "); 
    r = input_routine_number(); //schedule_enumerator = r-1;

    delete_schedule_routine();
    
    schedule_enumerator = r-1;

    insert_schedule_routine();
    
    define_Schedule_date(0);
    delay(10); 
    sorting_date_time(0);

    Serial.println("Schedule Saved!! Date and Time:\n ");
    print_schedule_date_time(0);

    alarm_played=0;
    set_alarm(alarm_played);

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
    Serial.println(r);

    return r;
}

void delete_schedule_routine()  //enumerator fix
{ 
    Y = 1 + schedule_enumerator; 
    Serial.print("schedule_enumerator = "); Serial.println(schedule_enumerator);
    if (edit == 0)
    {
        Serial.println("Insert routine number to delete : "); r = input_routine_number(); Serial.println(r);
    }

    for (int k=r-1; k < Y-1; k++)
    {
        for(int l =0; l < X; l++)
        {
            Schedule_time[k][l] = Schedule_time[k+1][l];   //move all values higher than the value to edit
            Schedule_date[k][l] = Schedule_date[k+1][l];
        }

        medicine[module_number[k]] = medicine[module_number[k+1]];
        number_of_pills[module_number[k]] = number_of_pills[module_number[k+1]];
        module_number[k] = module_number [k+1];
        everyday_routine[k] = everyday_routine[k+1];

    }
    
    Schedule_time[Y-1][0] = 0; Schedule_time[Y-1][1] = 0;  //delete that routine number
    
    medicine[module_number[Y-1]] = "empty";
    number_of_pills[module_number[Y-1]] = 0;
    module_number[Y-1] = 0;
    everyday_routine[Y-1] = 0;

    schedule_enumerator--;

    if (!edit)  {print_schedule_date_time(0);}

}

void print_schedule_date_time(int add)
{
    for (int k = 0; k < schedule_enumerator + add; k++) 
    {
        Serial.print(k+1); Serial.print(" - "); Serial.print(medicine[module_number[k]]); Serial.print("\n   "); 

        if(everyday_routine[k] == 0)
        {
            int temp_schedule = ntp_week_day;
            ntp_week_day = weekdayselected[k];
            print_weekday(); 
            ntp_week_day = temp_schedule; 
        
            temp_schedule = ntp_month;
            ntp_month = Schedule_date[k][1];
            print_name_month();
            ntp_month = temp_schedule;
        
            // Print date...
    
            print2digits(Schedule_date[k][0]);  Serial.print("/");
            print2digits(Schedule_date[k][1]);  Serial.print("/");
            print2digits(Schedule_date[k][2]);  Serial.print(" "); 
        }
        else 
        {
            Serial.print("Everyday ,         ");
            print2digits(Schedule_date[k][0]);  Serial.print("/");
            print2digits(Schedule_date[k][1]);  Serial.print("/");
            print2digits(Schedule_date[k][2]);  Serial.print(" ");
        }
        
    
        // ...and time
    
        print2digits(Schedule_time[k][0]);  Serial.print(":");
        print2digits(Schedule_time[k][1]);  Serial.print(":");
        print2digits(0);  Serial.print(" ");

        Serial.print("  Module number: "); Serial.print(module_number[k]); Serial.print("   Number of Pills: "); Serial.println(number_of_pills[module_number[k]]);
    }
    Serial.println("\n------------------------------------------------------------");
}


void print_schedule_time(int add)
{
  //         --------------------     PRINT THE WHOLE ROUTINE    -----------------------------    
        Serial.println("This is your full routine time schedule:");

        for (int k = 0; k < (schedule_enumerator + add) ; k++) 
        {
            Serial.print(k+1); Serial.print(" - "); Serial.print(medicine[module_number[k]]); Serial.print("    Time: "); 
            
            print2digits(Schedule_time[k][0]);  Serial.print(":");
            print2digits(Schedule_time[k][1]);  Serial.print(":");
            print2digits(0);  Serial.print(" ");

            Serial.print("    Module number :"); Serial.print(module_number[k]); Serial.print("    Number of Pills: "); Serial.println(number_of_pills[module_number[k]]);
        }
        Serial.println("\n------------------------------------------------------------");
    //              ---------------------------------------------------------                  */
}


int photointerrupter()
{
    
    delayMicroseconds(Delay_time_photoint_Microseconds);  
    
    int light = analogRead(LDR);
    Serial.print("light : ");           Serial.println(light); 

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
            i++;  error_count++; check_time_elapsed_success =0; photo_over = 1;

            result =0;    //return to success variable on main
            return result;
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

    else if (t_current - t_photo_start > TIMEOUT_COUNTER  &&   check_time_elapsed_success==0)   //     CASE THAT NO PILL IS DISPENSED.    ----------------------------------
    {
        Serial.println("TIMEOUT FOR PILL DETECTION! \t NO PILL DISPENSED");  //TO ADD! TRY AGAIN TO DISPENSE PILL!
        Serial.print("current time - last event time > TIMEOUT = "); Serial.println(t_current - mytime[i]);
        Serial.print("t_current: ") ;          Serial.println(t_current);
        Serial.print("mytime["); Serial.print(i); Serial.print("] : ");  Serial.println(mytime[i]);

        result =2;     //return to success variable for verify_success
    }
    else if (t_current - t_photo_start > TIMEOUT_COUNTER && check_time_elapsed_success==1) //     CASE THAT ONLY 1 PILL IS DISPENSED.    ----------------------------------
    {
        result =1; photo_over = 1;
    }

    return result;


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
  

      if((t_current - t_0 >1000) && cycle_stage == 3)   //wait 2000ms                             //   STAGE 3
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



void selector_function_input()
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

void selector_function_dispense(int mod_num)
{
  switch (mod_num) {
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





void unlock_pill()   // Stepper motor operation
{
    for(int j=0; j<stepper_steps; j++){
        
        int motor_Speed = 3;   /*  TO BE INSERTED THE RIGHT motor speed value   */ 
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
        
        int motor_Speed = 3;   /*  TO BE INSERTED THE RIGHT motor speed value   */ 
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
    Servo1.write (85);

}
void close_gate()
{
    Servo1.write (65);
}



void send_email()
{
    digitalWrite(SIGNAL_TO_ESP, HIGH);
    Serial.println("Sending_Email_to_Caregiver!!");
    delay(500);
    digitalWrite(SIGNAL_TO_ESP, LOW);
    delay(400);
}

void print2digits(int number) 
{
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}


void print_date_time()             //----------------------  PRINTING    --------------
{
  print_date();
  // ...and time
  print_time();
}

void print_date()
{
  print_weekday();
  print_name_month();

  // Print date...

  print2digits(rtc.getDay());  Serial.print("/");
  print2digits(rtc.getMonth());  Serial.print("/");
  print2digits(rtc.getYear());  Serial.print(" ");
}

void print_time()
{

  print2digits(rtc.getHours());  Serial.print(":");
  print2digits(rtc.getMinutes());  Serial.print(":");
  print2digits(rtc.getSeconds());  Serial.println();
}

void print_weekday()
{
    switch(ntp_week_day) {
                         
                         case 0: Serial.print("Sunday   ");
                                 break;
                         case 1: Serial.print("Monday   ");
                                 break;
                         case 2: Serial.print("Tuesday  ");
                                 break;
                         case 3: Serial.print("Wednesday");
                                 break;
                         case 4: Serial.print("Thursday ");
                                 break;
                         case 5: Serial.print("Friday   ");
                                 break;
                         case 6: Serial.print("Saturday ");
                                 break;
                         case 7: Serial.print("Everyday ");
                                 break; 
                         default: 
                                Serial.print(" Error_weekday! ntp_week_day=");
                                Serial.print((int)ntp_week_day);Serial.print(" ");
                         }
    Serial.print(", "); 
}

void print_name_month()
{
    switch(ntp_month)   {
                         
                         case 1: Serial.print("January ");
                                 break;
                         case 2: Serial.print("February ");
                                 break;
                         case 3: Serial.print("March ");
                                 break;
                         case 4: Serial.print("April ");
                                 break;
                         case 5: Serial.print("May ");
                                 break;
                         case 6: Serial.print("June ");
                                 break;
                         case 7: Serial.print("July ");
                                 break;
                         case 8: Serial.print("August ");
                                 break;
                         case 9: Serial.print("September ");
                                 break;
                         case 10: Serial.print("October ");
                                 break;
                         case 11: Serial.print("November ");
                                 break;
                         case 12: Serial.print("December ");       
                         default: break;        
                         }
}



 //                         ----------------------------------------------
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



void conversion_epoch_to_Time_Date()   //-------------------------------------------
{
    int leap_days=0; 
    int leap_year_ind=0;

    // Add or substract time zone here. 
    epoch+=3600 ; //GMT +1: = +3600 seconds 
    unsigned int epoch_tmp = epoch;
    
    ntp_second = epoch_tmp%60;
    epoch_tmp /= 60;
    ntp_minute = epoch_tmp%60;
    epoch_tmp /= 60;
    ntp_hour  = epoch_tmp%24;
    epoch_tmp /= 24;
        
    days_since_epoch = epoch_tmp;      //number of days since epoch
    ntp_week_day = week_days[days_since_epoch%7];  //Calculating WeekDay
      //
    ntp_year = 1970+(days_since_epoch/365); // ball parking year, may not be accurate!
 //
    int i;
    for (i=1972; i<ntp_year; i+=4)      // Calculating number of leap days since epoch/1970
       if(((i%4==0) && (i%100!=0)) || (i%400==0)) leap_days++;
          
    ntp_year = 1970+((days_since_epoch - leap_days)/365); // Calculating accurate current year by (days_since_epoch - extra leap days)
    day_of_year = ((days_since_epoch - leap_days)%365)+1;

    if(((ntp_year%4==0) && (ntp_year%100!=0)) || (ntp_year%400==0))  
     {
       month_days[1]=29;     //February = 29 days for leap years
       leap_year_ind = 1;    //if current year is leap, set indicator to 1 
      }
          else month_days[1]=28; //February = 28 days for non-leap years 

          temp_days=0;
   
    for (ntp_month=0 ; ntp_month <= 11 ; ntp_month++) //calculating current Month
       {
           if (day_of_year <= temp_days) break; 
           temp_days = temp_days + month_days[ntp_month];
        }
    
    temp_days = temp_days - month_days[ntp_month-1]; //calculating current Date
    ntp_day = day_of_year - temp_days;
  
    
    rtc.setTime(ntp_hour, ntp_minute, ntp_second);
    rtc.setDate(ntp_day, ntp_month, ntp_year - 2000);

    //print_date_time();
    //Serial.print("YEAR: ");
    //Serial.println(ntp_year);

    //Serial.print("Days since Epoch: "); Serial.println(days_since_epoch);
    //Serial.print("Number of Leap days since EPOCH: "); Serial.println(leap_days);
    //Serial.print("Day of year = "); Serial.println(day_of_year);
    //Serial.print("Is Year Leap? "); Serial.println(leap_year_ind);
    Serial.print("===============================================\n");
}



// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) 
{
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}



void debug_print_all_routine_variables()
{
    
    Serial.print("SCHEDULE ENUMERATOR: ");Serial.println(schedule_enumerator);

    print_schedule_date_time(0);
    for(int f=0; f<=schedule_enumerator; f++)
    {    
    
        Serial.print("Schedule_date["); Serial.print(f);  Serial.print("][2] = "); Serial.println(Schedule_date[f][2]);
        Serial.print("Schedule in minutes unordered!["); Serial.print(f);  Serial.print("][0] = "); Serial.println(schedules_in_minutes[f][0]);
    
        Serial.print("Schedule in minutes ordered!["); Serial.print(f);  Serial.print("][0] = "); (schedules_in_minutes[ordered_index[f]][0]);
        Serial.print("Ordered_index : "); Serial.println(ordered_index[f]);

    }
    

}


void EEPROM_SIM_write(int add)
{
    int k;
     // If the EEPROM is empty then isValid() is false
  if (!EEPROM.isValid()) 
  {

    Serial.println("EEPROM is empty, writing schedule data:");
    Serial.print("->");
    
    for (k = 0; k < schedule_enumerator + add; k++) 
    {
        for(int s=0; s<2; s++)
        {
            EEPROM.write(k, Schedule_time[k][s]);
            Serial.print(" ");
            Serial.print(Schedule_time[k][s]);
        }
        if (!everyday_routine[k])
        {
            for(int s=0; s<3; s++)
            {
                EEPROM.write(k+schedule_enumerator, Schedule_date[k][s]);
                Serial.print(" ");
                Serial.print(Schedule_date[k][s]);
            }
        }
        EEPROM.write(k+ 2*schedule_enumerator, module_number[k]);
    }


    Serial.println();

    // commit() saves all the changes to EEPROM, it must be called
    // every time the content of virtual EEPROM is changed to make
    // the change permanent.
    // This operation burns Flash write cycles and should not be
    // done too often. See readme for details:
    // https://github.com/cmaglie/FlashStorage#limited-number-of-writes
    EEPROM.commit();
    Serial.println("Done!");

    Serial.print("After commit, calling isValid() returns ");
    Serial.println(EEPROM.isValid());

  } 
  else 
  {

    Serial.println("EEPROM has been written.");
    Serial.println("Here is the content of the first 20 bytes:");

    Serial.print("->");
    for (int k = 0; k < 3*schedule_enumerator + add; k++) 
    {
      Serial.print(" ");
      Serial.print(EEPROM.read(k));
    }
    Serial.println();

  }
}

void EEPROM_SIM_read(int add)
{

    Serial.println("EEPROM has been written.");
    Serial.println("Here is the content of the first 3*schedule_enumerator bytes:");

    Serial.print("->");
    for (int k = 0; k < 3*schedule_enumerator + add; k++) 
    {
      Serial.print(" ");
      Serial.print(EEPROM.read(k));
    }
    Serial.println();
}
