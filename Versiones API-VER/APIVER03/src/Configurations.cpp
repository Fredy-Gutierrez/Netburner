/*
 * Configurations.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include "libs/Default_Values.h"
#include "libs/Configurations.h"
#include <config_server.h>
#include <config_obj.h>
#include <dhcpclient.h>
#include <netinterface.h>
#include <nbstring.h>
#include <stdlib.h>
#include <syslog.h>
#include <hal.h>

static config_string gIp { appdata, "", "SIp" };//IP var controller(saved in the flash memory)
static config_string gUrl { appdata, "", "SBaseUrl" };//IP var controller(saved in the flash memory)

static config_string gUdpIp { appdata, "", "IUdpIp" };//IP var controller(saved in the flash memory)
static config_int gUdpPort { appdata, 0, "IUdpPort" };//IP var controller(saved in the flash memory)

static config_int gTagsCounter { appdata, 1, "ITagsCounter" };//IP var controller(saved in the flash memory)



/*********************************************CHANGES THE NETBURNER VALS TO THE DEFAULT VALS***********************************************/
void ChangeToStaticMode()
{
    int ifNumber = GetFirstInterface();   // First interface is normally Ethernet 0

    InterfaceBlock *pIfBlock = GetInterfaceBlock(ifNumber);

    pIfBlock->ip4.mode = "Static";//change the ip mode to static (the default val is DHCP)

    pIfBlock->ip4.addr = AsciiToIp4(DEFAULTIP);//change the netburner's ip
    pIfBlock->ip4.mask = AsciiToIp4(DEFAULTMASK);//change the netburner's mask
    pIfBlock->ip4.gate = AsciiToIp4(DEFAULTGATEWAY);//change the netburner's gateway
    pIfBlock->ip4.dns1 = AsciiToIp4(DEFAULTDNS1);//change the netburner's dns

    gIp = DEFAULTSARGASIP;

    SaveConfigToStorage();//save the config
}


/*********************************************GET FROM THE FLASH MEMORY***********************************************/
const char* getSargasIp(){

	NBString baseIp = gIp.c_str();
	if(baseIp.empty()){

		gIp = DEFAULTSARGASIP;

		SaveConfigToStorage();

		return DEFAULTSARGASIP;
	}
	return gIp.c_str();
}

const char* getServerUrl(){
	NBString baseUrl = gUrl.c_str();
	if(baseUrl.empty()){

		gUrl = DEFAULTSERVERURL;

		SaveConfigToStorage();

		return DEFAULTSERVERURL;
	}
	return gUrl.c_str();
}

int getUdpPort(){
	return int(gUdpPort);
}

const char* getUdpIp(){
	return gUdpIp.c_str();
}

int getTagCounter(){
	return int(gTagsCounter);
}


/********************************************SET TO FLASH MEMORY***********************************************/
void setUdpIp(const char * newUdpIp){
	gUdpIp = newUdpIp;

	SaveConfigToStorage();
}

void setUdpPort(int port){
	gUdpPort = port;

	SaveConfigToStorage();
}

void incrementTagCounter(){
	int actualTag = int(gTagsCounter);

	if(actualTag <= 99998){
		gTagsCounter = actualTag + 1;

		SaveConfigToStorage();
	}
}

void setServerUrl(const char * newUrl){
	gUrl = newUrl;

	SaveConfigToStorage();
}

void setSargasIp(const char * newIp){
	gIp = newIp;

	SaveConfigToStorage();
}


/********************************************EXTERNAL FUNTION TO RESET THE SYSTEM, AND REBOOT THE SYSTEM***********************************************/
void restoreDefault(){

	ChangeToStaticMode();
	SysLog("The recovery mode has finished, you must remove the bridge the system will reset in 5 sec");

	//OSTimeDly(TICKS_PER_SECOND * 5);
	//ForceReboot();
}

/*
 * config_int gPort { appdata, 0, "Port" };//CASE: PORT CHANGE
void setSargasPort(const char * newPort){
	gPort = atoi(newPort); //parseInt

	iprintf("New value: %d\r\n", int(gPort));

	SaveConfigToStorage();
}*/






