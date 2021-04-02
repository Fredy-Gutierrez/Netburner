/*
 * Configurations.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include "libs/Default_Values.h"
#include "libs/Configurations.h"
#include <config_obj.h>
#include <config_server.h>
#include <dhcpclient.h>
#include <netinterface.h>
#include <nbstring.h>
#include <stdlib.h>
#include <syslog.h>
#include <hal.h>

static config_string gIp { appdata, "", "SIp" };

void ChangeToStaticMode() {
	int ifNumber = GetFirstInterface(); // First interface is normally Ethernet 0

	InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNumber);

	pIfBlock->ip4.mode = "Static";

	pIfBlock->ip4.addr = AsciiToIp4(DEFAULTIP);
	pIfBlock->ip4.mask = AsciiToIp4(DEFAULTMASK);
	pIfBlock->ip4.gate = AsciiToIp4(DEFAULTGATEWAY);
	pIfBlock->ip4.dns1 = AsciiToIp4(DEFAULTDNS1);

	SaveConfigToStorage();
}

const char* getSargasIp() {
	return gIp.c_str();
}

void setSargasIp(const char * newIp) {
	gIp = newIp;

	iprintf("New value: %s\r\n", gIp.c_str());

	SaveConfigToStorage();
}

void restoreDefault() {

	ChangeToStaticMode();
	SysLog(
			"The recovery mode has finished, you must remove the bridge the system will reset in 5 sec");

	OSTimeDly(TICKS_PER_SECOND * 5);
	ForceReboot();
}

/*
 * config_int gPort { appdata, 0, "Port" };//en caso de usar puertos dinamicos
 void setSargasPort(const char * newPort){
 gPort = atoi(newPort); //parseInt

 iprintf("New value: %d\r\n", int(gPort));

 SaveConfigToStorage();
 }*/

