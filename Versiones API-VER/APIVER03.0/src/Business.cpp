/*
 * Business.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 *
 */
#include <stdio.h>
#include <constants.h>
#include <nbrtos.h>
#include "libs/Business.h"
#include "libs/Tcp_Class.h"
#include "libs/I2C_Controller.h"
#include "libs/IO_Interface.h"
#include "libs/Rtc_Controller.h"

void displayMenu() {
	iprintf("\r\n*****Main Menu *****\r\n");
	iprintf("[I] Display current Inputs\r\n");
	iprintf("[H] Display current rtc time\r\n");
	iprintf("\r\n");
}

void commandTask(void *pd) {
	while (1) {
		char command = getchar();
		iprintf("\r\n");

		switch (command) {
		case 'H':
			printRtcTime();
			break;
		case 'I':
			printInputs();
			break;
		default:
			displayMenu();
		}
	}
}

void terminalCommand() {
	OSSimpleTaskCreatewName(commandTask, MAIN_PRIO + 4, "CommandTask");
}

void initBusiness() {
	initIO();
	initTcp();
	initI2C_Controller();
	terminalCommand();
}

