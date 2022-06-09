/*
 * Recovery_Mode.cpp
 *
 *  Created on: 5 mar 2021
 *      Author: Integra Fredy
 */
#include <config_obj.h>
#include <serial.h>
#include <startnet.h>
#include <stdio.h>
#include <string.h>
#include <config_server.h>
#include "libs/Recovery_Mode.h"
#include "libs/Default_Values.h"
#include "libs/Configurations.h"
#include <syslog.h>
#include <hal.h>

#define BAUDRATE_TO_USE (115200)
#define STOP_BITS (1)
#define DATA_BITS (8)

#include <config_obj.h>

char buffer[40];


/*********************************************STARTS THE PROCCESS TO CHECK IF THERE IS A JUMPER IN RX AND TX***********************************************/
BOOL initRecovery(){
	//BOOL result;
	char strTest[100];

	SerialClose(0);

	int uartFd = OpenSerial(0, BAUDRATE_TO_USE, STOP_BITS, DATA_BITS, eParityNone);

	sprintf(strTest,"\r\n\r\n\r\nBooting ......... checking recovery mode for version %s", APPVERSION);
	writestring(uartFd, strTest);

	char buffer[50];

	int n = ReadWithTimeout(uartFd, buffer, 50,3);

	buffer[n-1] = '\0';

	ReplaceStdio(0, uartFd);
	ReplaceStdio(1, uartFd);
	ReplaceStdio(2, uartFd);

	/*
	if(n>0 && strcmp(buffer,strTest)==0){
		restoreDefault();
		getAllLocalConfiguration();
		iprintf("\nRecovery mode. Using an saving factory default configuration, for AEI: %s", getLocalAeiID());
		result = true;

	}else{
		getAllLocalConfiguration();
		iprintf("\nNo recovery mode. Using configuration from flash memory, for AEI: %s", getLocalAeiID());
		result = false;
	}
	return result;
	*/
	getConfigurations();
	//checkEth0Config();
	return TRUE;
}


extern MonitorRecord monitor_config;//flash var that have control of debug port
/*********************************************CHECK IF THE DEBUG PORT IS IN UART1 IF NOT CHANGES IT***********************************************/
void checkDebugUart(){
	int debugUart = int(monitor_config.Uart);
	if(debugUart == 0){
		iprintf("\n\nChanging debug port to 1");
		//monitor_config.Uart = DEBUGPORT;
		SaveConfigToStorage();
		ForceReboot();
	}
}



