/*
 * IO_Interface.cpp
 *
 *  Created on: 22 sep 2021
 *      Author: Integra Fredy
 */
#include "headers/IO_Interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <pins.h>
#include <pin_irq.h>
#include <HiResDelay.h>
#include <same70.h>
#include <core_cm7.h>

extern "C" {
	void askForService(uint32_t format, void * obj, bool savCmd);
}

enum typeIoState {stIni, stStart, stScan};

typeIoState ioState;
DigDataBase digDB;

/**********************************NOTE********************************
 * If you add more digitals inputs you must modify the Dedaul_Values.h file,
 * you need to add the new values for close and open digitals inputs,
 * set to CLOSEDIGITALIN and OPENDIGITALIN macros the new positions of close and open digitals
 * in digitalsIn array
 * If you delete or add a new command input change the value of CMDDIGITALSIN in Dedaul_Values.h file adding the new commands inputs number
 * */
Digitals digitalsIn[MAX_DIG_INPUTS] = { { 2,22,FALSE,"Fase 1" }, {2,6,FALSE, "Fase 2"}, {2,7,FALSE, "Fase 3"}, {2,8,FALSE, "Cerrar" }, {2,9,FALSE,"_Abrir"} };
Digitals digitalsOut[MAX_DIG_OUTPUTS] = { {1,7,FALSE, "Cerrar"}, {1,6,FALSE,"_Abrir"} };

void AeiIoScan(){
	switch(ioState){
		case stIni:
			break;
		case stStart:
			ioState=stScan;
			break;
		case stScan:
			for(int i=0; i<MAX_DIG_OUTPUTS; i++){
				digDB.digOutputs[i].update();
			}
			for(int i=0; i<MAX_DIG_INPUTS; i++){
				digDB.digInputs[i].update();
			}
			break;
		default:
			break;
	}
}

void initIO(){
	ioState=stIni;
	digDB.progPins();
}
void ioStart(){
	ioState=stStart;
}
void ioStop(){
	ioState=stIni;
}

void setIOPulse(int ioDig, int ms){
	digDB.setOnPulse(ioDig, ms);
}
void printDigitals(){
	iprintf("******************Digitales******************\r\n");
	for(int i=0; i < MAX_DIG_INPUTS; i++){
		printf("Entrada %s\t estado: %d\n", digDB.digInputs[i].alias, (int)digDB.digInputs[i].state);
	}
}
void getDigitalsInfo(Digitals * digitals){
	for(int i=0; i<MAX_DIG_INPUTS; i++){
		memcpy(digitals[i].alias, digDB.digInputs[i].alias, ALIASDIGITALSIZE);
		digitals[i].pin = (uint8_t)digDB.digInputs[i].bit;
		digitals[i].report = 0;
	}
}

/***************************************************** <DataBase Class> ***********************************************/
DigDataBase::DigDataBase(){
	progPins();
}
void DigDataBase::progPins(){
	for(int i=0; i<MAX_DIG_INPUTS; i++){
		digDB.digInputs[i].construct((uint32_t)i, digitalsIn[i].port, digitalsIn[i].pin, digitalsIn[i].alias);
		digDB.digInputs[i].repChg=digitalsIn[i].report;
	}
	for(int i=0; i<MAX_DIG_OUTPUTS; i++){
		digDB.digOutputs[i].construct((uint32_t)i, digitalsOut[i].port, digitalsOut[i].pin,digitalsOut[i].alias );
		digDB.digOutputs[i].repChg=digitalsOut[i].report;
	}
	P1[OUTPUTS_PIN_ENABLE]=1;
}
void DigDataBase::updatePrnChange(uint32_t bit, bool prnChange){
	if(bit==ALL_DIGS){
		for(uint32_t i=0; i<MAX_DIG_INPUTS; i++){
			digDB.digInputs[i].repChg=prnChange;
		}
	}else if(bit>=0 && bit<MAX_DIG_INPUTS){
		digDB.digInputs[bit].repChg=prnChange;
	}
}
void DigDataBase::susRepChg(uint32_t dig, void (* fnChgReport)(DigInput * dig)){
	digInputs[dig].userChgReport = fnChgReport;
}
int DigDataBase::getDigState(uint32_t bit){
	return digInputs[bit].state;
}
void DigDataBase::setOnPulse(int outPut, int miliseconds){
	digDB.digOutputs[outPut].control(outPulseON, miliseconds);
}
/***************************************************** </DataBase Class> ***********************************************/

/***************************************************** <DigOutput Class> ***********************************************/
DigOutput::DigOutput(){
	state=outStOFF;
	valueField=NOT_UPDATED;
	function=outNoFunction;
	ticks=0;
	repChg=FALSE;
	userChgReport=0;
}
void DigOutput::construct(uint32_t _bit, uint32_t _port, uint32_t _pin, char* aliasDig){
	bit=_bit;
	port=_port;
	pin=_pin;
	memcpy(alias, aliasDig, ALIASDIGITALSIZE);
	//strcpy(alias, aliasDig);
	if(port == 1){
		P1[pin].function(PinIO::PIN_FN_OUT);
		P1[pin]=0;
	}else{
		P2[pin].function(PinIO::PIN_FN_OUT);
		P2[pin]=0;
	}
}
void DigOutput::control(typeOutFunction fun, uint32_t time){
	switch(fun){
		case outNoFunction:
			state=outStOFF;
			break;
		case outLatchON:
			state=outStON;
			break;
		case outLatchOFF:
			state=outStOFF;
			break;
		case outPulseON:
			state=outStPulseON;
			ticks=time*5;
			ticksCount=ticks;
			break;
		case outPulseOFF:
			state=outStPulseOFF;
			ticks=time*5;
			ticksCount=ticks;
			break;
	}
}
void DigOutput::update(){
	uint32_t newValue;
	switch(state){
		case outStOFF: default:
			newValue=0;
			break;
		case outStON:
			newValue=1;
			break;
		case outStPulseON:
			if(ticksCount-->0)
				newValue=1;
			else{
				newValue=0;
				state=outStOFF;
			}
			break;
		case outStPulseOFF:
			if(ticksCount-->0)
				newValue=0;
			else{
				newValue=1;
				ticksCount=ticks;
				state=outStPulseON;
			}
			break;
	}
	if(port==1){
		if(newValue==0)	P1[pin]=0; else P1[pin]=1;
	}else{
		if(newValue==0)	P2[pin].clr(); else P2[pin].set();
	}
}
/***************************************************** </DigOutput Class> ***********************************************/


/***************************************************** <DigInput Class> *************************************************/
DigInput::DigInput(){
	stateField=NOT_UPDATED;
	stateLast=NOT_UPDATED;
	state=NOT_UPDATED;
	filterTicks=0;
	repChg=FALSE;
	userChgReport=0;
}

void DigInput::construct(uint32_t _bit, uint32_t _port, uint32_t _pin, char* aliasDig){
	bit=_bit;
	port=_port;
	pin=_pin;
	memcpy(alias, aliasDig, ALIASDIGITALSIZE);
	if(port == 1){
		P1[pin].function(PinIO::PIN_FN_IN);
	}
	else{
		P2[pin].function(PinIO::PIN_FN_IN);
	}
}

void DigInput::update(){
	stateLast = stateField;
	//Update from field
	if(port == 1)
		stateField = (uint32_t)P1[pin].readBack();
	else
		stateField = (uint32_t)P2[pin].readBack();

	if(stateLast == stateField){
		if(++filterTicks == (uint32_t)FILTER_TICKS){
			if(state != stateField){
				if(state != NOT_UPDATED){
					state = stateField;
					getElapseTime(&chgTime);
					if(userChgReport!=0){
						userChgReport(this);
					}
				}else{
					state = stateField;
				}
			}
		}else if(filterTicks>(uint32_t)FILTER_TICKS){
			filterTicks=(uint32_t)FILTER_TICKS;
		}
	}else{
		filterTicks=0;
	}
}

struct InputsInfo *getDigitals(){
	struct InputsInfo * digitals = (struct InputsInfo *)malloc(sizeof(struct InputsInfo) * MAX_DIG_INPUTS);
	for(int i=0; i<MAX_DIG_INPUTS; i++){
		digitals[i].alias = digDB.digInputs[i].alias;
		digitals[i].inputNumber = digDB.digInputs[i].bit + 1;
		digitals[i].state = digDB.digInputs[i].state;
	}
	return digitals;
}

/*************************************** SET FUNCTIONS TO ATTEND ALL PROCESS **********************************/
void setFnsAttnProcess(uint32_t digNumber, void (* fn)(DigInput * dig)){
	digDB.susRepChg(digNumber, fn);
}
/***************************************************** </DigInput Class> *************************************************/
