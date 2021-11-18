#include <Servo.h> //Sservo motor library

Servo servo_gate;
int motorPin1 = 8;      //IN1 attached to pin 8    
int motorPin2 = 9;      //IN2 attached to pin 9
int motorPin3 = 10;     //IN3 attached to pin 10
int motorPin4 = 11;     //IN4 attached to pin 11
int motor_Speed = 4;    // stepper speed must be calibrated

void setup() {
	servo_gate.attach(1); //digital pin 1 is attached to the servo
	pinMode(motorPin1, OUTPUT); //setting pinmodes as outputs
  	pinMode(motorPin2, OUTPUT);
  	pinMode(motorPin3, OUTPUT);
  	pinMode(motorPin4, OUTPUT);
}

void dispense_pills() {

	//needs to be calibrated based on the strength the stepper applies on the pills
	//stepper turns right to lock all the pills stacked above the second
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
  	delay(500);
	
	servo_gate.write(-180); //servo turns 180 dgs to open the gate
	delay(1000); //pill drops
	servo_gate.write(0); //gate closes
	delay(1000);

	// stepper turns right to unlock the pills and let them fall on the gate
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
