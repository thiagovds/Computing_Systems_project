#define LDR A1


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
	2 (default) - dispense pills+
	+
	+

	*/
	Serial.println("Please insert desired mode! press:");
	Serial.println("0 - for inserting a schedule"); Serial.println("1 - for adding pills to a module storage"); Serial.println("any key - for dispensing pills");

	mode = Serial.read();  //get input from user in serial for mode, to be upgraded to a web service with ESP

	switch (mode) {
	    case 0:
	      Serial.println("You are in insert schedule mode! \n Please insert the hours '00' followed by an enter and then minutes '00' "); 
	      Serial.println("insert hours, range from 0 until 23:"); time[0] =Serial.read();
	      Serial.print("insert minutes, range from 0 until 59");  time[1] =Serial.read();

	      insert_schedule(*time, module_number, medicine);  //module number corresponds to a type of medicine
	      break;

	    case 1:
	      add_pills();
	      break;

	    default:
	      dispense_pills()
	}

}

int photointerrupter()
{

/* insert photointerrupter code here*/

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

