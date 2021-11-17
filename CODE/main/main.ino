#define LDR A1


int mode;
int time[];

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
	*/
	Serial.println("Please insert desired mode! press:");
	Serial.println("0 - for inserting a schedule"); Serial.println("1 - for adding pills to a module storage"); Serial.println("any key - for dispensing pills");

	Serial.read(mode);  //get input from user for mode

	switch (mode) {
	    case 0:
	      Serial.println("You are in insert schedule mode! \n Please insert the time '00:00' for the ");
	      insert_schedule(time, module_number, medicine);  //module number corresponds to a type of medicine
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
