#define LDR A1
#define stepMotorPin1 10
#define stepMotorPin2 11
#define stepMotorPin3 12
#define stepMotorPin4 13

/*  INITIALIZATION OF VARIABLES FOR MAIN  ----------------------  */


int mode;
int motor_Speed = 4



/*-----------------------*/



void setup()
{
	pinMode(LDR, INPUT);

  	pinMode(stepMotorPin1, OUTPUT);
  	pinMode(stepMotorPin2, OUTPUT);
  	pinMode(stepMotorPin3, OUTPUT);
  	pinMode(stepMotorPin4, OUTPUT);

}

void loop()
{
	/*DEFINE FINITE STATE MACHINE: MODES
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

	      insert_schedule();  
	      break;

	    case 1:
	      add_pills();
	      break;

	    default:
	      dispense_pills()
	}

	/*  
		
		INSERT CODE HERE AFTER MENU

	 */

}



void add_pills()
{
	string medicine;
	int number_of_pills;
	int module_number;

	Serial.println("You are in add pills mode! \n Please insert the module number that will receive the pill!");
	while (Serial.available() == 0) {} 	module_number = Serial.parseInt();

	Serial.println("Please insert the number of pills that will be added to the storage :");
	while (Serial.available() == 0) {} 	number_of_pills = Serial.parseInt();

	Serial.print("Please type the medicine name that will be added to the storage number" ); Serial.print(module_number); Serial.println(":");
	while (Serial.available() == 0) {}   S erial.setTimeout(5000);   //This sets the maximum time to wait for serial data from user.
	while (Serial.available() == 0) {}    medicine = Serial.readString();
	
}

void insert_schedule()   //TO BE ALTERED! DATA OF SCHEDULE HAS TO RETURN TO THE MAIN!!!
{
	int time[];
	int module_number;


	Serial.println("You are in insert schedule mode! \n Please insert the hours '00' followed by an enter and then minutes '00' "); 
	Serial.println("insert hours, range from 0 until 23:");  while (Serial.available() == 0) {}  time[0] =Serial.parseInt();
	Serial.println("insert minutes, range from 0 until 59"); while (Serial.available() == 0) {}  time[1] =Serial.parseInt();
	Serial.println("insert module number to be operated at the specified time:") while (Serial.available() == 0) {} module_number = Serial.parseInt();

	Serial.println("Would you like to insert the pills into the storage now? \n Y for yes, anykey for no.");
	while (Serial.available() == 0) {
  	}
  	Serial.setTimeout(1000);   //This sets the maximum time to wait for serial data from user.
  	string menuChoice = Serial.readString();
	
	if (!strcmp(menuChoice, "Y"))
	{
		add_pills();
	} 
	else
	{
		return
	}

}

int photointerrupter()
{

/* insert photointerrupter code here*/


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
	/* INSERT CODE FOR DISPENSING PILLS ALGORITHM HERE */
}

void pass_data_to_ESP()
{
	
}

