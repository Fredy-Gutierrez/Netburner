/*
 * Business.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 *
 */
#include <arp.h>
#include <netinterface.h>
#include <stdlib.h>
#include <stdio.h>

#include "libs/Business.h"
#include "libs/Tcp_Class.h"
#include "libs/I2C_Controller.h"
#include "libs/IO_Interface.h"
#include "libs/Rtc_Controller.h"




/*****************************************MENU FOR THE TERMINAL (SERIAL PORT)******************************/
void displayMenu()
{
    iprintf("\r\n*****Main Menu *****\r\n");
    iprintf("[H] Display current rtc time\r\n");
    iprintf("[I] Display current Inputs\r\n");
    iprintf("\r\n");
}


/*****************************************GETS THE COMMAND AND PROCCESS THE PRINTS******************************/
void commandTask(void *pd){
	displayMenu();
	while(true){

		char c = getchar();
		iprintf("\r\n");
		switch (c)
		{
			case 'H':
				printRtcTime();
				break;
			case 'I':
				getInputDiff();
				iprintf("inputs loaded\n");
				break;
			case 'S':
				startInterruption();
				break;
			default :
				displayMenu();
				break;
		}

	}
}


/*****************************************STARTS THE TASK THAT PRINT THE SYSTEM INFORMATIONS (LIKE INPUTS, HOUR AND MORE)******************************/
void terminalCommand(){
	OSSimpleTaskCreatewName(commandTask,MAIN_PRIO - 1 ,"CommandTask");
}


/*****************************************STARTS ALL THE SYSTEM'S PROCCESS******************************/
void initBusiness(){
	initTcp();
	initIO();
	initI2C_Controller();
	terminalCommand();
}

