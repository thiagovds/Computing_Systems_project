#define LDR A1



/*  INITIALIZATION OF VARIABLES FOR MAIN 


*/


int mode;
int time[2] = {0 , 0};

void setup()
{
	pinMode(LDR, INPUT);

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

	mode = Serial.read();  //get input from user in serial for mode, to be upgraded to a web service with ESP

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
	char medicine[];
	int number of pills;
	
	Serial.println("You are in add pills mode! \n Please insert the number of pills :");
	Serial.read();
}

void insert_schedule()   //TO BE ALTERED! DATA OF SCHEDULE HAS TO RETURN TO THE MAIN!!!
{
	int time[];
	int module_number;


	Serial.println("You are in insert schedule mode! \n Please insert the hours '00' followed by an enter and then minutes '00' "); 
	Serial.println("insert hours, range from 0 until 23:");   time[0] =Serial.read();
	Serial.println("insert minutes, range from 0 until 59");  time[1] =Serial.read();
	Serial.println("insert module number to be operated at the specified time:") module_number = Serial.read();

	Serial.println("Would you like to insert the pills into the storage now? \n Y for yes, anykey for no.");
	if (Serial.read(); == 'Y')
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
	/* INSERT CODE FOR DISPENSING PILLS ALGORITHM HERE (sound buzz) */
}

void pass_data_to_ESP()
{
	
}

