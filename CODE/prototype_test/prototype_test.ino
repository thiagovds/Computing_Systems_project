#include <Servo.h> //Inserire la libreria Servo

Servo Servo1;          //Servo is named Servo1
int button_1 = 8;      //trigger button on pin 8
int motorPin1 = 2;     //IN1 on pin 2   
int motorPin2 = 3;     //IN2 on pin 3
int motorPin3 = 4;     //IN3 on pin 4
int motorPin4 = 5;     //IN4 on pin 5

int motor_Speed = 5;   //stepper timing for stability
int val1 = 0;          //value of the button
int steps = 20;        //number of steps

void setup() {
  pinMode(button_1, INPUT);  
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
  Servo1.write(65);  // IMPORTANT: starting position of the servo
  Servo1.attach (6); // servo connected to pin 6 
}

void loop() {
  
  val1 = digitalRead(button_1);  //trigger button pressed

  if(val1 == HIGH){
    stepper_right();
    delay(1000);
    servo_right();
    delay(1000);
    servo_left();
    delay(1000);
    stepper_left();
    delay(1000);
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
  Servo1.write (90);
}

void servo_left(){
  Servo1.write (65);
}
