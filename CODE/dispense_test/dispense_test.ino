#include <Servo.h> 

#define Time_interval_stepper 1000
#define Time_interval_servo 500
#define stepper_steps 50

#define stepMotorPin1 10
#define stepMotorPin2 11
#define stepMotorPin3 12
#define stepMotorPin4 13

Servo Servo1;

int cycle_stage;
int start = 1;
void setup()
{
    Servo1.attach (9); //Il Servo1 è collegato al pin digitale 

    pinMode(stepMotorPin1, OUTPUT);
    pinMode(stepMotorPin2, OUTPUT);
    pinMode(stepMotorPin3, OUTPUT);
    pinMode(stepMotorPin4, OUTPUT);
    
    Serial.begin(9600);
    Serial.println("serial Begin!");

}

void loop()
{
    unsigned long t_0, t_current;
    if (start ==1) 
    {
        t_0 = millis; start = 0;
        cycle_stage = 1;
        Serial.println("cycle_stage 1 enabled");
    }

    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIME TO CURRENT ARD TIME  */
    t_current = millis();

    
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
