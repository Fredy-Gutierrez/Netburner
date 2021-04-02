/*
 * Recovery_Mode.cpp
 *
 *  Created on: 5 mar 2021
 *      Author: Integra Fredy
 */
#include <config_obj.h>
#include <serial.h>
#include <config_server.h>
#include <startnet.h>
#include <stdio.h>
#include <string.h>
#include "libs/Recovery_Mode.h"
#include "libs/Default_Values.h"
#include "libs/Configurations.h"
#include <syslog.h>
#include <hal.h>

#define RX_BUFSIZE (4096)
#define BAUDRATE_TO_USE (115200)
#define STOP_BITS (1)
#define DATA_BITS (8)

#include <config_obj.h>

char buffer[40];

BOOL initRecovery() {
	BOOL result;

	SerialClose(0);

	int uartFd = OpenSerial(0, BAUDRATE_TO_USE, 1, 8, eParityNone);

	writestring(uartFd, APPVERSION);

	char buffer[40];

	int n = ReadWithTimeout(uartFd, buffer, 40, 3);

	buffer[n] = '\0';

	ReplaceStdio(0, uartFd);
	ReplaceStdio(1, uartFd);
	ReplaceStdio(2, uartFd);

	if (n > 0) {
		iprintf("\nRecovery mode\n");
		restoreDefault();
		result = true;
	} else {
		iprintf("\nNo Recovery mode\n");
		result = false;
	}

	return result;
}

extern MonitorRecord monitor_config;
void checkDebugUart() {
	int debugUart = int(monitor_config.Uart);
	if (debugUart == 0) {
		iprintf("changing port");
		monitor_config.Uart = DEBUGPORT;
		SaveConfigToStorage();
		ForceReboot();
	}
}

