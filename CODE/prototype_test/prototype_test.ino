#include <Servo.h> //Inserire la libreria Servo
#define LDR A0

Servo Servo1;          //Servo is named Servo1

int button_1 = 8;      //trigger button on pin 8
int val1 = 0;          //value of the button

int motorPin1 = 2;     //IN1 on pin 2   
int motorPin2 = 3;     //IN2 on pin 3
int motorPin3 = 4;     //IN3 on pin 4
int motorPin4 = 5;     //IN4 on pin 5
int motor_Speed = 3;   //stepper timing for stability
int steps = 20;        //number of steps

int cal_value = 980;   //threshold value of the laser which is still not picking the pills passing
int light = 0;         //sensor value
int counter = 0;       //counter

void setup() {
  pinMode(button_1, INPUT);
  pinMode(LDR, INPUT);  
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  Servo1.attach (6);          //servo connected to pin 6
  Servo1.write(65);           // IMPORTANT: starting position of the servo
  Serial.begin(9600);
}

void loop() {
  
  val1 = digitalRead(button_1);  //trigger button pressed

  if(val1 == HIGH){
    stepper_right();
    delay(500);
    servo_right();

    light = analogRead(LDR);
  
    if(light < cal_value){
      counter = counter + 1;
      Serial.print("Sensor value is: ");
      Serial.println(light);
      Serial.print("Pill dispensed: ");
      Serial.println(counter);
    }
    
    delay(800);
    servo_left();
    delay(500);
    stepper_left();
  }

}

void stepper_left(){
  for(int i=0; i<steps; i++){
    digitalWrite(motorPin4, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, LOW);
    delay(motor_Speed);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, LOW);
    delay(motor_Speed);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin1, LOW);
    delay(motor_Speed);
    digitalWrite(motorPin4, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin1, HIGH);
    delay(motor_Speed);
  }
}

void stepper_right(){
  for(int j=0; j<steps; j++){
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    delay(motor_Speed);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, LOW);
    delay(motor_Speed);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, HIGH);
    digitalWrite(motorPin4, LOW);
    delay(motor_Speed);
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    digitalWrite(motorPin3, LOW);
    digitalWrite(motorPin4, HIGH);
    delay(motor_Speed);
  }
}

void servo_right(){
  Servo1.write (85);
}

void servo_left(){
  Servo1.write (65);
}
