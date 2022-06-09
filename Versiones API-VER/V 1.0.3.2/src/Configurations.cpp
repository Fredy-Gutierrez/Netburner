/*
 * Configurations.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
/************************************************
 * *************LAST UPDATE 2021-10-15***********
 * **********************************************
 * 	1.- ADDED THE POSSIBILITY OF HAVING MORE THAN ONE RFID READER
 * 		1.1.- ADDED THE gQtySargas VARIABLE
 * 		1.2.- THE VARIABLE gSargasIp HAS BEEN DELETED
 * 		1.3.- ADDED THE VARIABLES gSargasIpR AND gSargasIpL
 * 		1.4.- ADDED THE VARIABLES gQtySargasAntR AND gQtySargasAntL
 * 	2.- THE VARIABLE char gBattInstallDate2[12] HAS BEEN DELETED
 * 	3.- ADDED THE POSSIBILITY OF HAVING MORE AXLES DETECTORS
 * 		3.1.- THE VARIABLE gADSignal HAS CHANGED TO gMainADSignal WHICH IS THE SIGNAL FOR MAIN AXLES DETECTOR
 * 		3.2.- THE VARIABLE gDoubleAxDet HAS CHANGED TO gDoubleAxlesDetector WHICH IS THE VARIABLE TO ENABLE THE SECOND AXLES DETECTOR
 * 		3.3.- ADDED THE VARIABLE gDoubleADSignal WHICH IS THE SIGNAL FOR SECOND AXLES DETECTOR
 * 		3.4.- ADDED THE VARIABLE gMirrorAxDet WHICH IS THE VARIABLE TO ENABLE THE MIRROR AXLES DETECTOR
 * 		3.5.- ADDED THE VARIABLE gMirrorADSignal WHICH IS THE SIGNAL FOR MIRROR AXLES DETECTOR
 * 	4.- ADDED MORE DEFAULT CONFIGURATIONS MACROS FOR NEW VARIABLES ADDED IN CONFIGURATION STRUCT
 * 	5.- CHANGE THE CONFIGURATION SAVE METHOD, IN LAST VERSIONS WE SAVE THE CONFIGURATIONS FOR EACH OF THE VARIABLES, NOW AFTER
 * 		MAKE A CHANGES IN THE CONFIGURATIONS YOU MUST CALL THE FUNTION saveSettings() TO SAVE YOUR CONFIGURATIONS, THIS ALLOWS MAKE A LOT
 * 		OF CHANGES IN THE VARIABLES AND AFTER THIS SAVE IT INTO NON-VOLATILE MEMORY, AND THE SAVE METHOD WILL BE MORE EFECTIVE AND FASTER THAN BEFORE
 *
 * **********************************************/
#include "libs/Default_Values.h"
#include "libs/Configurations.h"
#include <hal.h>
#include <ctype.h>
#include <nbrtos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <syslog.h>
#include <ipshow.h>
#include <dhcpclient.h>
#include <ipv6/ipv6_interface.h>
#include <netinterface.h>
#include <config_server.h>
#include <nbtime.h>

BOOL bCheckedNV = false;

// *** WARNING: CHANGE THE VERIFY KEY ANY TIME YOU MAKE A CHANGE TO THIS STRUCTURE ***
struct NV_SettingsStruct
{
    uint32_t VerifyKey;    // Changes when the structure is modified, so we can detect the change
    uint32_t StructSize;   // Store structure size
    /*******************************CONNECTIVITIES*******************/
    //MAC30 IP-ADMIN
    char gNbIpMode[8];
    char gNbIp[16];
    char gNbIpMask[16];
    char gNbIpGate[16];
    char gNbIpDns1[16];
    //SERVER REPORT
	char gServerUrl[100];
	//LOG INFORMATION
	uint8_t gLogChoice;
	char gUdpIp[16];
	uint32_t gUdpPort;
	//RFID SARGAS INFORMATION
	uint8_t gQtySargas;
	char gSargasIpR[16];
	char gSargasIpL[16];
	uint8_t gQtySargasAntR;
	uint8_t gQtySargasAntL;
	/*******************************GPIO*******************/
    uint8_t gSys1;
	uint8_t gSys1p;
	uint8_t gSys1n;
	uint8_t gSys1np;

	uint8_t gSys2;
	uint8_t gSys2p;
	uint8_t gSys2n;
	uint8_t gSys2np;

	uint8_t gLoopC_I;
	uint8_t gLoopO;
	uint8_t gDcOk1;
	uint8_t gDcOk2;
	uint8_t gAcOk1;
	uint8_t gBat1;
	uint8_t gAcOk2;
	uint8_t gBat2;
	uint8_t gDoor;
	uint8_t gFree;
	/*******************************GENERAL*******************/
	char gAeiID[8];
	bool gAeiDir;
	uint8_t gLoopType;
	uint16_t gLoopTimeOn;
	uint16_t gLoopTimeOff;
	bool gLoopRfidOnOff;
	bool gNtpSyncOnOff;
	char gNtpServer[50];
	uint8_t gNtpSyncTime;
	bool gBattTestOnOff;
	uint8_t gBattTestPeriod;
	uint8_t gBattTestHour;
	long long gBattInstallDate;//change
	long long gBattTestLast;//new

	uint8_t gMainADSignal;
	bool gDoubleAxlesDetector;
	uint8_t gDoubleADSignal;
	int gRDirectionADouble;
	bool gMirrorAxlesDetector;
	uint8_t gMirrorADSignal;
	int gRDirectionAMirror;
	char gPhoneNumber[15];
	uint8_t gTempAlarmSet;
	/*******************************ADDITIONAL*******************/
	uint8_t gStatusReportingTime;
	uint8_t gAeiStatus;
	uint32_t gCarReportedCounter;
	uint32_t gTrainCounter;
}__attribute__((packed));

NV_SettingsStruct Settings;

void setDefaultSettings(){
	strcpy(Settings.gNbIpMode, DEFAULT_NB_IP_MODE);
	strcpy(Settings.gNbIp, DEFAULT_NB_IP_ADD);
	strcpy(Settings.gNbIpMask, DEFAULT_NB_IP_MASK);
	strcpy(Settings.gNbIpGate, DEFAULT_NB_IP_GATEWAY);
	strcpy(Settings.gNbIpDns1, DEFAULT_NB_IP_DNS1);
	Settings.gSys1 = DEFAULT_IN_SYS1;
	Settings.gSys1p = DEFAULT_IN_SYS1P;
	Settings.gSys1n = DEFAULT_IN_SYS1_N;
	Settings.gSys2 = DEFAULT_IN_SYS2;
	Settings.gSys2p = DEFAULT_IN_SYS2P;
	Settings.gSys1np = DEFAULT_IN_SYS1_NP;
	Settings.gSys2n = DEFAULT_IN_SYS2_N;
	Settings.gSys2np = DEFAULT_IN_SYS2_NP;
	Settings.gLoopC_I = DEFAULT_IN_LOOPC_I;
	Settings.gLoopO = DEFAULT_IN_LOOPO;
	Settings.gDcOk1 = DEFAULT_IN_DCOK1;
	Settings.gDcOk2 = DEFAULT_IN_DCOK2;
	Settings.gAcOk1 = DEFAULT_IN_ACOK1;
	Settings.gBat1 = DEFAULT_IN_BAT1;
	Settings.gAcOk2 = DEFAULT_IN_ACOK2;
	Settings.gBat2 = DEFAULT_IN_BAT2;
	Settings.gDoor = DEFAULT_IN_DOOR;
	Settings.gFree = DEFAULT_IN_FREE;
	strcpy(Settings.gSargasIpR, DEFAULT_SARGAS_IP_R);
	strcpy(Settings.gSargasIpL, DEFAULT_SARGAS_IP_L);
	strcpy(Settings.gServerUrl, DEFAULT_SERVER_URL);
	strcpy(Settings.gUdpIp, DEFAULT_UDP_IP);
	Settings.gUdpPort = DEFAULT_UDP_PORT;
	Settings.gLogChoice = DEFAULT_LOG_OUT;
	strcpy(Settings.gAeiID, DEFAULT_AEI_ID);
	Settings.gLoopType = DEFAULT_LOOP_TYPE;
	Settings.gLoopTimeOn = DEFAULT_LOOP_TIME_ON;
	Settings.gLoopTimeOff = DEFAULT_LOOP_TIME_OFF;
	Settings.gLoopRfidOnOff = DEFAULT_LOOP_RFID_ONOFF;
	Settings.gAeiDir = DEFAULT_AEI_DIR;
	Settings.gNtpSyncOnOff = DEFAULT_NTP_SYNC_ONOFF;
	strcpy(Settings.gNtpServer, DEFAULT_NTP_SERVER_NAME);
	Settings.gNtpSyncTime = DEFAULT_NTP_SYNC_TIME;
	Settings.gBattTestOnOff = DEFAULT_BATT_TEST_ONOFF;
	Settings.gBattTestPeriod = DEFAULT_BATT_TEST_PERIOD;
	Settings.gBattTestHour = DEFAULT_BATT_TEST_HOUR;
	Settings.gTempAlarmSet = DEFAULT_TEMP_ALARM_SET;
	Settings.gMainADSignal = DEFAULT_MAIN_AXIES_DET_SIGNAL;
	Settings.gDoubleAxlesDetector = DEFAULT_DOUBLE_AXIES_DET;
	Settings.gDoubleADSignal = DEFAULT_DOUBLE_AXIES_DET_SIGNAL;
	Settings.gRDirectionADouble = DEFAULT_DIR_DOUBLE_AD;
	Settings.gMirrorAxlesDetector = DEFAULT_MIRROR_AXIES_DET;
	Settings.gMirrorADSignal = DEFAULT_MIRROR_AXIES_DET_SIGNAL;
	Settings.gRDirectionAMirror = DEFAULT_DIR_MIRROR_AD;
	Settings.gQtySargas = DEFAULT_QTY_SARGAS;
	Settings.gQtySargasAntR = DEFAULT_QTY_SARGAS_ANT_R;
	Settings.gQtySargasAntL = DEFAULT_QTY_SARGAS_ANT_L;
	strcpy(Settings.gPhoneNumber, DEFAULT_PHONE_NUMBER);
	Settings.gCarReportedCounter = 1;
	Settings.gTrainCounter = 1;
	Settings.gBattInstallDate = DEFAULT_BATT_INSTALL_DATE;//change
	Settings.gBattTestLast = DEFAULT_BATT_TEST_LAST;//new
}

void CheckNVSettings()
{
    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    if ( (pData->StructSize != sizeof(Settings)) || (pData->VerifyKey != VERIFY_KEY))
    {
    	/*THIS CONDITION WILL TAKE EFFECT WHEN,
    	 *  YOU MAKE A CHANGE IN THE STRUCT SIZE OR VERIFY_KEY OR IF THE STRUCT DON´T EXIST IN FLASH,
    	 *  SO IF YOU ADD/REMOVE A VARIABLE, THIS if() WILL EXEC*/

        Settings.VerifyKey = VERIFY_KEY;//SETTING THE NEW VERIFY_KEY
        /*******************************************WARNING****************************************
         * WHEN YOU MAKE A CHANGE IN THE STRUCT SIZE
         * you can choice between set the default values or set the data stored in flash,
         * but with last you must have caution because
         * you can´t set data which isn´t in the flash.
         * for example, if you add a new variable but you had values in the flash which you want assign at new struct,
         * you can assign it using the pointer pData->, but you must not do it for the new variable added because
         * in the flash those values don´t exist,
         * you must do it only for the existing variables in flash*/

        /*----------------------------------------To Assign default values uncomment the next line----------------------------------*/
        setDefaultSettings();

        Settings.StructSize = sizeof(NV_SettingsStruct);
        SaveUserParameters(&Settings, sizeof(NV_SettingsStruct));
    }
    else
    {
    	/*------------------------------- Assign values from flash to settings structure (local values)-----------------------------------*/
        Settings.VerifyKey = pData->VerifyKey;

        /*------------------------------- NB IP CONFIG -----------------------------------*/
        strcpy(Settings.gNbIpMode, pData->gNbIpMode);
        strcpy(Settings.gNbIp, pData->gNbIp);
        strcpy(Settings.gNbIpGate, pData->gNbIpGate);
        strcpy(Settings.gNbIpMask, pData->gNbIpMask);
        strcpy(Settings.gNbIpDns1, pData->gNbIpDns1);
        /*------------------------------- GPIO CONFIG -----------------------------------*/
        Settings.gSys1 = pData->gSys1;
        Settings.gSys1p = pData->gSys1p;
        Settings.gSys1n = pData->gSys1n;
        Settings.gSys2 = pData->gSys2;
		Settings.gSys2p = pData->gSys2p;
		Settings.gSys1np = pData->gSys1np;
		Settings.gSys2n = pData->gSys2n;
		Settings.gSys2np = pData->gSys2np;
		Settings.gLoopC_I = pData->gLoopC_I;
		Settings.gLoopO = pData->gLoopO;
		Settings.gDcOk1 = pData->gDcOk1;
		Settings.gDcOk2 = pData->gDcOk2;
		Settings.gAcOk1 = pData->gAcOk1;
		Settings.gAcOk2 = pData->gAcOk2;
		Settings.gBat1 = pData->gBat1;
		Settings.gBat2 = pData->gBat2;
		Settings.gDoor = pData->gDoor;
		Settings.gFree = pData->gFree;
		/*------------------------------- CONECTIVITIES CONFIG -----------------------------------*/
		Settings.gQtySargas = pData->gQtySargas;
		strcpy(Settings.gSargasIpR, pData->gSargasIpR);
		Settings.gQtySargasAntR = pData->gQtySargasAntR;
		strcpy(Settings.gSargasIpL, pData->gSargasIpL);
		Settings.gQtySargasAntL = pData->gQtySargasAntL;
		strcpy(Settings.gServerUrl, pData->gServerUrl);
		Settings.gLogChoice = pData->gLogChoice;
		strcpy(Settings.gUdpIp, pData->gUdpIp);
		Settings.gUdpPort = pData->gUdpPort;
		/*------------------------------- GENERAL CONFIG -----------------------------------*/
        strcpy(Settings.gAeiID, pData->gAeiID);
        Settings.gAeiDir = pData->gAeiDir;
        Settings.gLoopType = pData->gLoopType;
        Settings.gLoopTimeOn = pData->gLoopTimeOn;
        Settings.gLoopTimeOff = pData->gLoopTimeOff;
        Settings.gLoopRfidOnOff = pData->gLoopRfidOnOff;
        Settings.gNtpSyncOnOff = pData->gNtpSyncOnOff;
        strcpy(Settings.gNtpServer, pData->gNtpServer);
        Settings.gNtpSyncTime = pData->gNtpSyncTime;
        Settings.gBattTestOnOff = pData->gBattTestOnOff;
        Settings.gBattTestPeriod = pData->gBattTestPeriod;
        Settings.gBattTestHour = pData->gBattTestHour;
        Settings.gMainADSignal = pData->gMainADSignal;
        Settings.gDoubleAxlesDetector = pData->gDoubleAxlesDetector;
        Settings.gDoubleADSignal = pData->gDoubleADSignal;
        Settings.gRDirectionADouble = pData->gRDirectionADouble;
        Settings.gMirrorAxlesDetector = pData->gMirrorAxlesDetector;
        Settings.gMirrorADSignal = pData->gMirrorADSignal;
        Settings.gRDirectionAMirror = pData->gRDirectionAMirror;
        Settings.gBattInstallDate = pData->gBattInstallDate;
        Settings.gBattTestLast = pData->gBattTestLast;
        /*------------------------------- ADITIONAL CONFIG -----------------------------------*/
        Settings.gTempAlarmSet = pData->gTempAlarmSet;
        strcpy(Settings.gPhoneNumber, pData->gPhoneNumber);
        Settings.gAeiStatus = pData->gAeiStatus;
        Settings.gStatusReportingTime = pData->gStatusReportingTime;
        Settings.gCarReportedCounter = pData->gCarReportedCounter;
        Settings.gTrainCounter = pData->gTrainCounter;

        Settings.StructSize = pData->StructSize;
    }
    bCheckedNV = TRUE;
}
/************************************************
 * GETTERS AREA*
 * **********************************************/

/**************************************************GETTERS FOR CONNECTIVITIES CONFIGURATIONS*************************************/

int getQtySargas(){
	return Settings.gQtySargas;
}
const char* getQtySargasStr(){
	if(Settings.gQtySargas == 0 ){
		return "SINGLE";
	}else if(Settings.gQtySargas == 1){
		return "DOUBLE'";
	}
	return "UNKNOWN";
}
const char * getSargasIp(char id){
	switch(id){
		case 'R': case 'r':
			return Settings.gSargasIpR;
		case 'L': case 'l':
			return Settings.gSargasIpL;
		default:
			return NULL;
	}
}
int getQtySargasAnt(char id){
	switch(id){
		case 'R':case 'r':
			return Settings.gQtySargasAntR;
			break;
		case 'L':case 'l':
			return Settings.gQtySargasAntL;
			break;
	}
	return 0;
}

const char * getLogChoiceStr(){
	if(Settings.gLogChoice == 0){
		return "NONE";
	}else if(Settings.gLogChoice == 1){
		return "SERIAL";
	}else if(Settings.gLogChoice == 2){
		return "UDP";
	}else if(Settings.gLogChoice == 3){
		return "BOTH";
	}
	return "UNKNOWN";
}
const char * getUdpIp(){ return Settings.gUdpIp; }
int getUdpPort(){ return Settings.gUdpPort; }
int getLogChoice(){ return Settings.gLogChoice; }

const char * getServerUrl(){ return Settings.gServerUrl; }

const char * getCurrentNbIpMode(){
	const char * s = "DHCP";
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	if(pIfBlock->ip4.mode.IsSelected(s)){
		return s;
	}else{
		return "Static";
	}
}
char ip[16];
const char * getCurrentNbIpAddress(){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	((IPADDR4)(pIfBlock->ip4.cur_addr)).sprintf(ip, 16);
	return ip;
}
char mask[16];
const char * getCurrentNbIpMask(){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	((IPADDR4)(pIfBlock->ip4.cur_mask)).sprintf(mask, 16);
	return mask;
}
char gate[16];
const char* getCurrentNbIpGate(){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	((IPADDR4)(pIfBlock->ip4.cur_gate)).sprintf(gate, 16);
	return gate;
}
char dns[16];
const char* getCurrentNbIpDns1(){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	((IPADDR4)(pIfBlock->ip4.cur_dns1)).sprintf(dns, 16);
	return dns;
}

const char * getNbIpMode(){
	return Settings.gNbIpMode;
}
const char * getNbIpAddress(){
	if(strcmp(Settings.gNbIpMode, "DHCP") == 0){
		return getCurrentNbIpAddress();
	}
	return Settings.gNbIp;
}
const char * getNbIpMask(){
	if(strcmp(Settings.gNbIpMode, "DHCP") == 0){
		return getCurrentNbIpMask();
	}
	return Settings.gNbIpMask;
}
const char* getNbIpGate(){
	if(strcmp(Settings.gNbIpMode, "DHCP") == 0){
		return getCurrentNbIpGate();
	}
	return Settings.gNbIpGate;
}
const char* getNbIpDns1(){
	if(strcmp(Settings.gNbIpMode, "DHCP") == 0){
		return getCurrentNbIpDns1();
	}
	return Settings.gNbIpDns1;
}

/**************************************************GETTERS FOR GPIO CONFIGURATIONS*************************************/
void updateIOMap(idxIoSignals index, int digitalNum){
	tyIoSignal newInput;
	newInput.refDigInput = digitalNum;
	newInput.bit = digitalNum - 1;
	setDigInputsBit(newInput, index);
}

int getInputByIndex(idxIoSignals index){
	switch(index){
		case idxSys1:
			return Settings.gSys1;
		case idxSys2:
			return Settings.gSys2;
		case idxSysP1:
			return Settings.gSys1p;
		case idxSysP2:
			return Settings.gSys2p;
		case idxSysN1:
			return Settings.gSys1n;
		case idxSysN2:
			return Settings.gSys2n;
		case idxSysPN1:
			return Settings.gSys1np;
		case idxSysPN2:
			return Settings.gSys2np;
		case idxLoopC_I:
			return Settings.gLoopC_I;
		case idxLoopO:
			return Settings.gLoopO;
		case idxDcOk1:
			return Settings.gDcOk1;
		case idxDcOk2:
			return Settings.gDcOk2;
		case idxAcOk1:
			return Settings.gAcOk1;
		case idxAcOk2:
			return Settings.gAcOk2;
		case idxBat1:
			return Settings.gBat1;
		case idxBat2:
			return Settings.gBat2;
		case idxDoor:
			return Settings.gDoor;
		case idxFree:
			return Settings.gFree;
		default:
			return -1;
	}
}
void updateAllMap(){
	updateIOMap(idxSys1, Settings.gSys1);
	updateIOMap(idxSys2, Settings.gSys2);
	updateIOMap(idxSysP1, Settings.gSys1p);
	updateIOMap(idxSysP2, Settings.gSys2p);
	updateIOMap(idxSysN1, Settings.gSys1n);
	updateIOMap(idxSysN2, Settings.gSys2n);
	updateIOMap(idxSysPN1, Settings.gSys1np);
	updateIOMap(idxSysPN2, Settings.gSys2np);
	updateIOMap(idxLoopC_I, Settings.gLoopC_I);
	updateIOMap(idxLoopO, Settings.gLoopO);
	updateIOMap(idxDcOk1, Settings.gDcOk1);
	updateIOMap(idxDcOk2, Settings.gDcOk2);
	updateIOMap(idxAcOk1, Settings.gAcOk1);
	updateIOMap(idxAcOk2, Settings.gAcOk2);
	updateIOMap(idxBat1, Settings.gBat1);
	updateIOMap(idxBat2, Settings.gBat2);
	updateIOMap(idxDoor, Settings.gDoor);
	updateIOMap(idxFree, Settings.gFree);
}
/**************************************************GETTERS FOR GENERAL CONFIGURATIONS*************************************/
const char * getAeiID(){ return Settings.gAeiID; }
int getAeiDir(){ if(Settings.gAeiDir){ return 1; } return -1; }
const char * getAeiDirStr(){ if(Settings.gAeiDir){ return "FORWARD"; }else{ return "BACKWARD"; } }
int getLoopType(){ return Settings.gLoopType; }
const char * getLoopTypeStr(){ if(Settings.gLoopType == 0){ return "SINGLE"; }else{ return "DOUBLE"; }; }
int getLoopTimeOn(){ return Settings.gLoopTimeOn; }
int getLoopTimeOff(){ return Settings.gLoopTimeOff; }
bool getLoopRfidOnOff(){ return Settings.gLoopRfidOnOff; }
bool getNtpSyncOnOff(){ return Settings.gNtpSyncOnOff; }
const char * getNtpServer(){ return Settings.gNtpServer; }
int getNtpSyncTime(){ return Settings.gNtpSyncTime; }
bool getBattTestOnOff(){ return Settings.gBattTestOnOff; }
long long getBattInstallDate(){ return Settings.gBattInstallDate; }
int getBattTestPeriod(){ return Settings.gBattTestPeriod; }
int getBattTestHour(){ return Settings.gBattTestHour; }
long long getBattTestLast(){ return Settings.gBattTestLast; }

int getMainADSignal(){
	return Settings.gMainADSignal;
}
idxIoSignals getMainADSignalIndex(){
	switch(Settings.gMainADSignal){
		case 0:
			return idxSys1;
		case 1:
			return idxSysP1;
		case 2:
			return idxSysN1;
		case 3:
			return idxSysPN1;
		default:
			return idxFree;
	}
}

bool getDoubleAxDet(){ return Settings.gDoubleAxlesDetector; }
int getDoubleADSignal(){ return Settings.gDoubleADSignal; }
int getDirDoubleAxDet(){ return Settings.gRDirectionADouble; }
idxIoSignals getDoubleADSignalIndex(){
	switch(Settings.gDoubleADSignal){
		case 0:
			return idxSys1_S;
		case 1:
			return idxSysP1_S;
		default:
			return idxFree;
	}
}

bool getMirrorAxDet(){ return Settings.gMirrorAxlesDetector; }
int getMirrorADSignal(){ return Settings.gMirrorADSignal; }
idxIoSignals getMirrorADSignalIndex(){
	switch(Settings.gMirrorADSignal){
		case 0:
			return idxSys1;
		case 1:
			return idxSysP1;
		case 2:
			return idxSysN1;
		case 3:
			return idxSysPN1;
		default:
			return idxFree;
	}
}
int getDirMirrorAxDet(){ return Settings.gRDirectionAMirror; }

int getTempAlarmSet(){ return Settings.gTempAlarmSet; }
const char * getPhoneNumber(){ return Settings.gPhoneNumber;  }

/**************************************************GETTERS FOR ADDITIONAL INFORMATION*************************************/
int getStatusReportingTime(){ return Settings.gStatusReportingTime; }
int getAeiStatus(){ return Settings.gAeiStatus; }
int getCarCounter(){ return Settings.gCarReportedCounter; }
int getTrainCounter(){ return Settings.gTrainCounter; }

/************************************************
 * SETTERS TO FLASH MEMORY*
 * **********************************************/

/**************************************************SETTERS FOR CONNECTIVITIES INFORMATION*************************************/
void setNbIpMode( char * value){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	pIfBlock->ip4.mode = value;
	strcpy(Settings.gNbIpMode,value);
}
void setNbIpAddress(char * value){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	pIfBlock->ip4.addr = AsciiToIp4(value);
	strcpy(Settings.gNbIp,value);
}
void setNbIpMask(char * value){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	pIfBlock->ip4.mask = AsciiToIp4(value);
	strcpy(Settings.gNbIpMask,value);
}
void setNbIpGate(char * value){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	pIfBlock->ip4.gate = AsciiToIp4(value);
	strcpy(Settings.gNbIpGate,value);
}
void setNbIpDns1(char * value){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	pIfBlock->ip4.dns1 = AsciiToIp4(value);
	strcpy(Settings.gNbIpDns1,value);
}

void setLogChoice(int val){
	Settings.gLogChoice = val;
}
void setUdpIp(char * newUdpIp){
	strcpy(Settings.gUdpIp,newUdpIp);
}
void setUdpPort(int port){
	Settings.gUdpPort = port;
}

void setServerUrl(char * newUrl){
	strcpy(Settings.gServerUrl, newUrl);
}

void setSargasQty(int val){ Settings.gQtySargas = val; }
void setSargasIpR(char * newIp){
	strcpy(Settings.gSargasIpR, newIp);
}
void setSargasIpL(char * newIp){
	strcpy(Settings.gSargasIpL, newIp);
}
void setSargasAntQtyR(int val){ Settings.gQtySargasAntR = val; }
void setSargasAntQtyL(int val){ Settings.gQtySargasAntL = val; }

/**************************************************SETTERS FOR GPIO INFORMATION*************************************/
void setInSys1(int newVal){ Settings.gSys1 = newVal; updateIOMap(idxSys1, newVal); }
void setInSys1p(int newVal){ Settings.gSys1p = newVal; updateIOMap(idxSysP1, newVal); }
void setInSys1n(int newVal){ Settings.gSys1n = newVal; updateIOMap(idxSysN1, newVal); }
void setInSys2(int newVal){ Settings.gSys2 = newVal; updateIOMap(idxSys2, newVal); }
void setInSys2p(int newVal){ Settings.gSys2p = newVal; updateIOMap(idxSysP2, newVal); }
void setInSys1np(int newVal){ Settings.gSys1np = newVal; updateIOMap(idxSysPN1, newVal); }
void setInSys2n(int newVal){ Settings.gSys2n = newVal; updateIOMap(idxSysN2, newVal); }
void setInSys2np(int newVal){ Settings.gSys2np = newVal; updateIOMap(idxSysPN2, newVal); }
void setInLoopC_I(int newVal){ Settings.gLoopC_I = newVal; updateIOMap(idxLoopC_I, newVal); }
void setInLoopO(int newVal){ Settings.gLoopO = newVal; updateIOMap(idxLoopO, newVal); }
void setInDcOk1(int newVal){ Settings.gDcOk1 = newVal; updateIOMap(idxDcOk1, newVal); }
void setInDcOk2(int newVal){ Settings.gDcOk2 = newVal; updateIOMap(idxDcOk2, newVal); }
void setInAcOk1(int newVal){ Settings.gAcOk1 = newVal; updateIOMap(idxAcOk1, newVal); }
void setInBat1(int newVal){ Settings.gBat1 = newVal; updateIOMap(idxBat1, newVal); }
void setInAcOk2(int newVal){ Settings.gAcOk2 = newVal; updateIOMap(idxAcOk2, newVal); }
void setInBat2(int newVal){ Settings.gBat2 = newVal; updateIOMap(idxBat2, newVal); }
void setInDoor(int newVal){ Settings.gDoor = newVal;  updateIOMap(idxDoor, newVal);}
void setInFree(int newVal){ Settings.gFree = newVal;  updateIOMap(idxFree, newVal);}

/**************************************************SETTERS FOR GENERAL INFORMATION*************************************/
void setAeiID(char * value){ strcpy(Settings.gAeiID,value); }
void setLoopType(int type){ Settings.gLoopType = type; }
void setLoopTimeOn(int time){ Settings.gLoopTimeOn = time; }
void setLoopTimeOff(int time){ Settings.gLoopTimeOff = time; }
void setLoopRfidOnOff(bool val){ Settings.gLoopRfidOnOff = val; }
void setAeiDir(bool val){ Settings.gAeiDir = val; }
void setNtpSyncOnOff(bool val){ Settings.gNtpSyncOnOff = val; }
void setNtpServer(char * value){ strcpy(Settings.gNtpServer,value); }
void setNtpSyncTime(int value){ Settings.gNtpSyncTime = value; }
void setBattTestOnOff(bool val){ Settings.gBattTestOnOff = val; }
void setBattInstallDate(long long value){ Settings.gBattInstallDate = value; }
void setBattTestPeriod(int value){ Settings.gBattTestPeriod = value; }
void setBattTestHour(int value){ Settings.gBattTestHour = value; }
void setBattTestLast(long long value){ Settings.gBattTestLast = value; saveSettings();}
void setMainADSignal(int value){ Settings.gMainADSignal = value; }
void setDoubleAxDet(bool value){ Settings.gDoubleAxlesDetector = value; }
void setDoubleADSignal(int value){ Settings.gDoubleADSignal = value; }
void setDirDoubleAxDet(int value){ Settings.gRDirectionADouble = value; }
void setMirrorAxDet(bool value){ Settings.gMirrorAxlesDetector = value; }
void setMirrorADSignal(int value){ Settings.gMirrorADSignal = value; }
void setDirMirrorAxDet(int value){ Settings.gRDirectionAMirror = value; }

void setTempAlarmSet(int value){ Settings.gTempAlarmSet = value; }
void setPhoneNumber(char * value){ strcpy(Settings.gPhoneNumber,value); }

/**************************************************SETTERS FOR ADDITIONAL INFORMATION*************************************/
void setStatusReportingTime(int value){ Settings.gStatusReportingTime = value; saveSettings();}
void setAeiStatus(int value){ Settings.gAeiStatus = value; saveSettings(); }
void incrementCarCounter(){
	if(Settings.gCarReportedCounter <= 99998){
		Settings.gCarReportedCounter++;
	}else{
		Settings.gCarReportedCounter = 1;
	}
	saveSettings();
}
void incrementTrainCounter(){
	if(Settings.gTrainCounter < 9999){
		Settings.gTrainCounter++;
	}else{
		Settings.gCarReportedCounter = 1;
	}
	saveSettings();
}
void saveSettings(){
	if (sizeof(Settings) < MAX_USER_PARAM_SIZE)
	{
		SaveUserParameters(&Settings, sizeof(Settings));
	}
	else
	{
		iprintf("*** ERROR: Your structure exceeds User Param Flash space, save aborted\r\n");
	}
}
void saveConfigToStorage(){ SaveConfigToStorage(); }

//Forces the system hardware to perform a soft reset
void restartMicroChip(){
	ForceReboot();
}

void changeToStaticMode(){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	if(pIfBlock->dhcpClient.GetDHCPState() != SDHCP_NOTSTARTED){
		pIfBlock->dhcpClient.StopDHCP();
	}
	pIfBlock->ip4.cur_addr = AsciiToIp4(getNbIpAddress());
	pIfBlock->ip4.cur_mask = AsciiToIp4(getNbIpMask());
	pIfBlock->ip4.cur_gate = AsciiToIp4(getNbIpGate());
	pIfBlock->ip4.cur_dns1 = AsciiToIp4(getNbIpDns1());
}
void changeToDHCPMode(){
	InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
	if(pIfBlock->dhcpClient.GetDHCPState() == SDHCP_NOTSTARTED){
		pIfBlock->dhcpClient.StartDHCP();
	}
}

void restartDHCP(){
	if(strcmp(Settings.gNbIpMode, "DHCP") == 0){
		InterfaceBlock *pIfBlock = GetInterfaceBlock(GetFirstInterface());
		pIfBlock->dhcpClient.RestartDHCP();
	}
}

void checkEth0Config(){
	if(strcmp(getCurrentNbIpMode(), getNbIpMode()) != 0){
		setNbIpMode(Settings.gNbIpMode);
		if(strcmp(Settings.gNbIpMode, "DHCP") == 0){
			//iprintf("************************Changing from Static to DHCP***********************");
			saveConfigToStorage();
			changeToDHCPMode();
		}else{
			//iprintf("************************Changing from DHCP to Static ***********************");
			setNbIpAddress(Settings.gNbIp);
			setNbIpMask(Settings.gNbIpMask);
			setNbIpGate(Settings.gNbIpGate);
			setNbIpDns1(Settings.gNbIpDns1);
			saveConfigToStorage();
			changeToStaticMode();
		}
		saveSettings();
		restartMicroChip();
	}
}

/*********************************************CHANGES THE NETBURNER VALS TO THE DEFAULT VALS*********************************************/
void ChangeToDefault()
{
	if (!bCheckedNV) CheckNVSettings();
	setDefaultSettings();
	setNbIpMode(DEFAULT_NB_IP_MODE);
	if(strcmp(DEFAULT_NB_IP_MODE,"Static") == 0){
		setNbIpAddress(DEFAULT_NB_IP_ADD);
		setNbIpMask(DEFAULT_NB_IP_MASK);
		setNbIpGate(DEFAULT_NB_IP_GATEWAY);
		setNbIpDns1(DEFAULT_NB_IP_DNS1);
		changeToStaticMode();
	}else{
		changeToDHCPMode();
	}
	saveSettings();
	saveConfigToStorage();
	restartMicroChip();
}
void getConfigurations(){
	if (!bCheckedNV) CheckNVSettings();
	updateAllMap();
	//checkEth0Config();
}

/******************************************EXTERNAL FUNTION TO RESET THE SYSTEM, AND REBOOT THE SYSTEM*******************************/
void restoreDefault(){
	ChangeToDefault();
	SysLog("The recovery mode has finished, \nyou must remove the bridge and wait 10 sec\n");
	OSTimeDly(TICKS_PER_SECOND * 10);
}

//recive date in format YYYY-MM-DD and return it in long long
long long convertDateToSec(const char * date){
	struct tm timeAsSec;
	char sdate[5];
	sdate[0] = date[0]; sdate[1] = date[1]; sdate[2] = date[2]; sdate[3] = date[3]; sdate[4] = '\0';
	timeAsSec.tm_year = atoi(sdate)-1900;
	memset(sdate,'\0',5);

	sdate[0] = date[5]; sdate[1] = date[6]; sdate[2] = '\0';
	timeAsSec.tm_mon = atoi(sdate)-1;
	memset(sdate,'\0',5);

	sdate[0] = date[8]; sdate[1] = date[9]; sdate[2] = '\0';
	timeAsSec.tm_mday = atoi(sdate);
	memset(sdate,'\0',5);

	timeAsSec.tm_hour = 0;
	timeAsSec.tm_min = 0;
	timeAsSec.tm_sec = 0;
	timeAsSec.tm_wday = 0;

	time_t t = timegm( &timeAsSec );

	return static_cast<long long>(t);
}

//return date in format YYYY-MM-DD
char dateConverted[12];
char * convertSecToAsciiDate(long long dateInSec){
	time_t t = dateInSec;
	struct tm *ptm = gmtime( &t );
	sprintf(dateConverted, "%04hd-%02hd-%02hd", (short)ptm->tm_year + 1900, (short)ptm->tm_mon + 1 , (short)ptm->tm_mday);
	return dateConverted;
}





