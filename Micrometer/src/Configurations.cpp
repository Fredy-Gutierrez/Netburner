/*
 * Configurations.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#include "headers/Default_Values.h"
#include "headers/Configurations.h"
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
    uint32_t VerifyKey;    // Changes when the structure is modified so we can detect the change
    uint32_t StructSize;   // Store structure size

    /*******************Horometer device***************/
    char hMaker[10];
	char hType[20];
	char hSerialNum[50];
	/*******************Device to be tasted***************/
	char dOperator[100];
	char dSubStation[100];
	char dDevice[100];
	char dSerialNum[50];
	char dMaker[100];
	char dCapacity[50];
	char dType[50];
	char dTension[50];
	uint32_t dReportNum;
	char dDivision[50];
	char dZone[100];
	char dDate[15];
	char dDateLastTest[15];
	/*******************Test information***************/
	uint8_t tInitialState;
	uint8_t tCycleNumber;
	uint8_t tCycleWaitTime;
	uint8_t tCmdWaitTime;
	uint16_t tClosePulseTime;
	uint16_t tOpenPulseTime;
	uint16_t tPhasesWaitTime;
	uint16_t tMaxDiffClose;
	uint16_t tMaxDiffOpen;
	uint16_t tMaxTimeAfterPulse;
} __attribute__((packed));

NV_SettingsStruct Settings;

void setDefaultSettings(){
	strcpy(Settings.hMaker,HMAKERDEFAULT);
	strcpy(Settings.hType,HTYPEDEFAULT);
	strcpy(Settings.hSerialNum,HSERIALDEFAULT);

	strcpy(Settings.dOperator,DOPERATORDEFAULT);
	strcpy(Settings.dSubStation,DSUBSTATIONDEFAULT);
	strcpy(Settings.dDevice,DDEVICEDEFAULT);
	strcpy(Settings.dSerialNum,DSERIALDEFAULT);
	strcpy(Settings.dMaker,DMAKERDEFAULT);
	strcpy(Settings.dCapacity,DCAPACITYDEFAULT);
	strcpy(Settings.dType,DTYPEDEFAULT);
	strcpy(Settings.dTension,DTENSIONDEFAULT);
	Settings.dReportNum = DREPORTNUMDEFAULT;
	strcpy(Settings.dDivision,DDIVISIONDEFAULT);
	strcpy(Settings.dZone,DZONEDEFAULT);
	strcpy(Settings.dDate,DDATEDEFAULT);
	strcpy(Settings.dDateLastTest,DDATELASTTESTDEFAULT);

	Settings.tInitialState = TINITIALSTATEDEFAULT;
	Settings.tCycleNumber = TCYCLENUMBERDEFAULT;
	Settings.tCycleWaitTime = TCYCLEWAITTIMEDEFAULT;
	Settings.tCmdWaitTime = TCMDWAITTIMEDEFAULT;
	Settings.tClosePulseTime = TCLOSEPULSETIMEDEFAULT;
	Settings.tOpenPulseTime = TOPENPULSETIMEDEFAULT;
	Settings.tPhasesWaitTime = TPHASESWAITTIMEDEFAULT;
	Settings.tMaxDiffClose = TMAXDIFFCLOSEDEFAULT;
	Settings.tMaxDiffOpen = TMAXDIFFOPENDEFAULT;
	Settings.tMaxTimeAfterPulse = TMAXTIMEAFTERPULSEDEFAULT;
}

void CheckNVSettings()
{
    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    if ((pData->VerifyKey != VERIFY_KEY) || (pData->StructSize != sizeof(Settings)))
    {
    	/*THIS CONDITION WILL TAKE EFFECT WHEN,
    	 *  YOU MAKE A CHANGE IN THE STRUCT SIZE OR VERIFY_KEY OR IF THE STRUCT DON´T EXIST IN FLASH,
    	 *  SO IF YOU ADD/REMOVE A VARIABLE, THIS if() WILL EXEC*/
        Settings.VerifyKey = VERIFY_KEY;//SETTING THE NEW VERIFY_KEY
        /*******************************************ATTENTION****************************************
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
        Settings.StructSize = sizeof(Settings);
        SaveUserParameters(&Settings, sizeof(Settings));
    }
    else
    {
    	/*------------------------------- Assign values from flash to settings structure (local values)-----------------------------------*/
        Settings.VerifyKey = pData->VerifyKey;

        strcpy(Settings.hMaker,pData->hMaker);
        strcpy(Settings.hType,pData->hType);
        strcpy(Settings.hSerialNum,pData->hSerialNum);

        strcpy(Settings.dOperator,pData->dOperator);
        strcpy(Settings.dSubStation,pData->dSubStation);
        strcpy(Settings.dDevice,pData->dDevice);
        strcpy(Settings.dSerialNum,pData->dSerialNum);
        strcpy(Settings.dMaker,pData->dMaker);
        strcpy(Settings.dCapacity,pData->dCapacity);
        strcpy(Settings.dType,pData->dType);
        strcpy(Settings.dTension,pData->dTension);
        Settings.dReportNum = pData->dReportNum;
        strcpy(Settings.dDivision,pData->dDivision);
        strcpy(Settings.dZone,pData->dZone);
        strcpy(Settings.dDate,pData->dDate);
        strcpy(Settings.dDateLastTest,pData->dDateLastTest);

        Settings.tInitialState = pData->tInitialState;
        Settings.tCycleNumber = pData->tCycleNumber;
        Settings.tCycleWaitTime = pData->tCycleWaitTime;
        Settings.tCmdWaitTime = pData->tCmdWaitTime;
        Settings.tClosePulseTime = pData->tClosePulseTime;

        Settings.tOpenPulseTime = pData->tOpenPulseTime;

        Settings.tPhasesWaitTime = pData->tPhasesWaitTime;
        Settings.tMaxDiffClose = pData->tMaxDiffClose;
        Settings.tMaxDiffOpen = pData->tMaxDiffOpen;
        Settings.tMaxTimeAfterPulse = pData->tMaxTimeAfterPulse;

        Settings.StructSize = pData->StructSize;
    }
    bCheckedNV = TRUE;
}
/******************************************Horometer information***************************************/
const char * getHMaker(){
	return Settings.hMaker;
}
const char * getHType(){
	return Settings.hType;
}
const char * getHSerialNum(){
	return Settings.hSerialNum;
}
/******************************************Device information***************************************/
const char * getDOperator(){
	return Settings.dOperator;
}
const char * getDSubStation(){
	return Settings.dSubStation;
}
const char * getDDevice(){
	return Settings.dDevice;
}
const char * getDSerialNum(){
	return Settings.dSerialNum;
}
const char * getDMaker(){
	return Settings.dMaker;
}
const char * getDCapacity(){
	return Settings.dCapacity;
}
const char * getDType(){
	return Settings.dType;
}
const char * getDTension(){
	return Settings.dTension;
}
uint32_t getDReportNum(){
	return Settings.dReportNum;
}
const char * getDDivision(){
	return Settings.dDivision;
}
const char * getDZone(){
	return Settings.dZone;
}
const char * getDDate(){
	return Settings.dDate;
}
const char * getDDateLastTest(){
	return Settings.dDateLastTest;
}
/******************************************Test information***************************************/
ProcessState getTInitialState(){
	return static_cast<ProcessState>(Settings.tInitialState);
}
uint8_t getTCycleNumber(){
	return Settings.tCycleNumber;
}
uint8_t getTCycleWaitTime(){
	return Settings.tCycleWaitTime;
}
uint8_t getTCmdWaitTime(){
	return Settings.tCmdWaitTime;
}
uint16_t getTClosePulseTime(){
	return Settings.tClosePulseTime;
}
uint16_t getTOpenPulseTime(){
	return Settings.tOpenPulseTime;
}
uint16_t getTPhasesWaitTime(){
	return Settings.tPhasesWaitTime;
}
uint16_t getTMaxDiffClose(){
	return Settings.tMaxDiffClose;
}
uint16_t getTMaxDiffOpen(){
	return Settings.tMaxDiffOpen;
}
uint16_t getTMaxTimeAfterPulse(){
	return Settings.tMaxTimeAfterPulse;
}
/******************************************Save settings into memory***************************************/
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
void setNewConfiguration(Configurations * pData){
	/*
	strcpy(Settings.hMaker,pData->hMaker);
	strcpy(Settings.hType,pData->hType);
	strcpy(Settings.hSerialNum,pData->hSerialNum);*/

	strcpy(Settings.dOperator,pData->dOperator);
	strcpy(Settings.dSubStation,pData->dSubStation);
	strcpy(Settings.dDevice,pData->dDevice);
	strcpy(Settings.dSerialNum,pData->dSerialNum);
	strcpy(Settings.dMaker,pData->dMaker);
	strcpy(Settings.dCapacity,pData->dCapacity);
	strcpy(Settings.dType,pData->dType);
	strcpy(Settings.dTension,pData->dTension);
	Settings.dReportNum = pData->dReportNum;
	strcpy(Settings.dDivision,pData->dDivision);
	strcpy(Settings.dZone,pData->dZone);
	strcpy(Settings.dDate,pData->dDate);
	strcpy(Settings.dDateLastTest,pData->dDateLastTest);

	Settings.tInitialState = pData->tInitialState;
	Settings.tCycleNumber = pData->tCycleNumber;
	Settings.tCycleWaitTime = pData->tCycleWaitTime;
	Settings.tCmdWaitTime = pData->tCmdWaitTime;
	Settings.tClosePulseTime = pData->tClosePulseTime;
	Settings.tOpenPulseTime = pData->tOpenPulseTime;
	Settings.tPhasesWaitTime = pData->tPhasesWaitTime;
	Settings.tMaxDiffClose = pData->tMaxDiffClose;
	Settings.tMaxDiffOpen = pData->tMaxDiffOpen;
	Settings.tMaxTimeAfterPulse = pData->tMaxTimeAfterPulse;
	saveSettings();
}


/***************************************************extern**********************************************/
void getConfigurations(){
	if (!bCheckedNV) CheckNVSettings();
}

//Forces the system hardware to perform a soft reset
void reset(){
	ForceReboot();
}









