#define Time_interval_stepper 1000
#define Time_interval_servo 500

#define stepMotorPin1 10
#define stepMotorPin2 11
#define stepMotorPin3 12
#define stepMotorPin4 13

int start = 1;
void setup()
{
    pinMode(stepMotorPin1, OUTPUT);
    pinMode(stepMotorPin2, OUTPUT);
    pinMode(stepMotorPin3, OUTPUT);
    pinMode(stepMotorPin4, OUTPUT);
    Serial.begin(9600);
    Serial.println("serial Begin!");

}

void loop()
{
    unsigned long t_0, t_1;
    int cycle_stage;

    if (start ==1) 
    {
        t_0 = millis; start = 0;
        cycle_stage = 1;
        Serial.println("cycle_stage 1 enabled");
        Serial.println(cycle_stage);
    }

    /* TAKE TIMESTAMP EVERY CYCLE i.e. UPDATE TIME TO CURRENT ARD TIME  */
    t_1 = millis();

    
    if((t_1 - t_0 >500) &&  (cycle_stage == 1))                                            //   STAGE 1
    {       
        lock_pill();
        t_0 = t_1;
        cycle_stage = 2;
        Serial.println("lock pill! Getting ready for Stage 2");
        Serial.println(cycle_stage);
    }
    Serial.println("END OF STAGE 1");

/*              ESSENTIAL FOR PHOTOINTERRUPTER TO WORK HERE                        */

  if((t_1 - t_0 >500) &&  cycle_stage == 2)   //wait 500ms                             //   STAGE 2
    {   
        open_gate();
        t_0 = t_1;
        cycle_stage = 3;
        Serial.println("open gate! Getting ready for Stage 3");
    }
  

/*                                                            */
    if((t_1 - t_0 >2000) && cycle_stage == 3)   //wait 2000ms                             //   STAGE 3
    {   
        close_gate();
        t_0 = t_1;
        cycle_stage = 4;
        Serial.println("close gate! Getting ready for Stage 4");
    }
    //wait 500ms

    if((t_1 - t_0 >500) && cycle_stage == 4 )  //wait 500ms                             //   STAGE 3
    {   
        unlock_pill();
        t_0 = t_1;
        cycle_stage = 1;
        Serial.println("unlock pill! Getting ready for Stage 1");
    }
    
    //wait 1000ms


}

void lock_pill()   // Stepper motor operation
{
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

void unlock_pill()   //Stepper_motor operation
{
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



void open_gate()
{

}

void close_gate()
{

}
