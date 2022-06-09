/*
 * IO_Interface.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include "libs/IO_Interface.h"
#include "libs/log.h"
#include "libs/Rtc_Controller.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"
#include <stdio.h>
#include <string.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <stopwatch.h>
#include <pins.h>
#include <pin_irq.h>
#include <iostream>
#include <HiResDelay.h>
#include <same70.h>
#include <core_cm7.h>

/*
enum idxIoSignals {idxSys1, idxSys2, idxSysP1, idxSysP2, idxSysN1, idxSysN2, idxSysPN1, idxSysPN2,
					idxLoopC_I, idxLoopO, idxDcOk1, idxDcOk2, idxAcOk1, idxAcOk2, idxBat1, idxBat2,
					idxDoor, idxFree};
typedef struct {
	const char * id;
	int refDigInput;
	uint32_t bit;
}tyIoSignal;
#define MAX_IO_SIGNALS		idxFree + 1
*/
tyIoSignal ioSignals[MAX_IO_SIGNALS] = {{"SYS1\0\0\0\0", 3, 2},{"SYS2\0\0\0\0", 17, 16},{"SYS1'\0\0\0", 2, 1},{"SYS2'\0\0\0", 7, 6},{"/SYS1\0\0\0", 1, 0},
										{"/SYS2\0\0\0", 5, 4},{"/SYS1'\0\0", 6, 5},{"/SYS2'\0\0", 10, 9},{"LoopC_I\0", 9, 8},{"LoopO\0\0\0", 8, 7},
										{"DcOk1\0\0\0", 13, 12},{"DcOk2\0\0\0", 12, 11},{"AcOk1\0\0\0", 11, 10},{"AcOk2\0\0\0", 15, 14},
										{"Bat1\0\0\0\0", 16, 15},{"Bat2\0\0\0\0", 14, 13},{"Door\0\0\0\0", 4, 3},{"Free\0\0\0\0", 18, 17}};

volatile unsigned long countTicks[8000];
volatile unsigned long qtyTicks=0;

extern "C" {void askForService(uint32_t format, void * obj, bool savCmd);}
extern logger logg;
digDataBase digDB;

enum typeIoState {stIni, stStart, stScan};
typeIoState ioState;

uint32_t cfgDigIn[MAX_DIG_INPUTS][3] = {{2,22,FALSE},{2,6,FALSE},{2,7,FALSE},{2,8,FALSE},{2,9,FALSE},{2,10,FALSE},{2,11,FALSE},
				{2,12,FALSE},{2,13,FALSE},{2,15,FALSE},{2,16,FALSE},{2,17,FALSE},{2,18,FALSE},{2,19,FALSE},{2,20,FALSE},
				{2,21,FALSE},{1,31,FALSE},{1,33,FALSE}};

uint32_t cfgDigOut[MAX_DIG_OUTPUTS][3] = {{1,7,FALSE},{1,6,FALSE}};

/*****************************************INIT THE CONFIGURATIONS******************************/
void initIO(){
	ioState=stIni;
	digDB.progPins();
}

void AeiIoScan(){
	switch(ioState){
	case stIni:
	default:
		break;
	case stStart:
		ioState=stScan;
		break;
	case stScan:
		for(int i=0; i<MAX_DIG_INPUTS; i++){
			digDB.digInputs[i].update();
		}
		for(int i=0; i<MAX_DIG_OUTPUTS; i++){
			digDB.digOutputs[i].update();
		}
		break;
	}
}

void ioStart(){
	stMac30.setStatus(idxMac30, 'N', 1);
	ioState=stStart;
}
void ioStop(){
	stMac30.setStatus(idxMac30, 'W', 2);
	ioState=stIni;
}
void ioPrnChgDisable(uint32_t bit){
	digDB.updatePrnChange(bit, FALSE);
}
void ioPrnChgEnable(uint32_t bit){
	digDB.updatePrnChange(bit, TRUE);
}

void ioDigOutputControl(uint32_t bit, typeOutFunction function, uint32_t time){
	digDB.digOutputs[bit].control(function, time);
}

void SaveTicks(unsigned long ticks){
	countTicks[qtyTicks++] = ticks;
	if(qtyTicks>8000)qtyTicks=0;
}

void printTicks(void){
	char strPrn[200];
	logg.prn("\nPrinting elapsed ticks on interruption by user request (8000 values)");
	for( int j=0; j < 8000; j++){
		sprintf(strPrn, "\n[%04d] %lu", j, (unsigned long)countTicks[j]); logg.prn(strPrn);
	}
}

void printDigitals(void){
	logg.newPrint("\nPrinting digital input values by user request:");
	for(int i=0; i<MAX_DIG_INPUTS; i++){
		logg.newPrint("\n   Digital input[%02d][%7s] state: %d;  P%d[%02d], time of last change: %llu.%03lu", i+1, ioGetDigInputId((int)i),
								(int)digDB.digInputs[i].state, (int)digDB.digInputs[i].port, (int)digDB.digInputs[i].pin, digDB.digInputs[i].chgTime.sysTime,
								digDB.digInputs[i].chgTime.msCurrSec);
	}
}

const char *ioGetDigInputId(uint32_t _bit){
	for(int i=idxSys1; i<=idxFree; i++){
		if(ioSignals[i].bit==_bit)
			return ioSignals[i].id;
	}
	return "NotFound\0";
}

uint32_t sgGetDigInputBit(int idx){
	if(idx>=idxSys1 && idx<=idxFree)
		return ioSignals[idx].bit;
	else
		return 100;
}

void setDigInputsBit(tyIoSignal newInput, idxIoSignals index){
	ioSignals[index].bit = newInput.bit;
	ioSignals[index].refDigInput = newInput.refDigInput;
}

/***************************************************** Class digDataBase **********************************************
*
class digDataBase {
public:
	digInput digInputs[MAX_DIG_INPUTS];

	digDataBase();
	void updatePrnChange(uint32_t bit, bool prnChange);
	void susRepChg(uint32_t dig, void (* fnChgReport)(digInput * dig));

};
*/

digDataBase::digDataBase(){
	progPins();
}

void digDataBase::progPins(){
	for(int i=0; i<MAX_DIG_INPUTS; i++){
		digDB.digInputs[i].construct((uint32_t)i, cfgDigIn[i][0], cfgDigIn[i][1]);
		digDB.digInputs[i].repChg=cfgDigIn[i][2];
	}
	for(int i=0; i<MAX_DIG_OUTPUTS; i++){
		digDB.digOutputs[i].construct((uint32_t)i, cfgDigOut[i][0], cfgDigOut[i][1]);
		digDB.digOutputs[i].repChg=cfgDigOut[i][2];
	}
	P1[OUTPUTS_PIN_ENABLE]=1;
}
void digDataBase::updatePrnChange(uint32_t bit, bool prnChange){
	if(bit==ALL_DIGS)
		for(uint32_t i=0; i<MAX_DIG_INPUTS; i++){
			digDB.digInputs[i].repChg=prnChange;
		}
	else if(bit>=0 && bit<MAX_DIG_INPUTS)
		digDB.digInputs[bit].repChg=prnChange;
}

void digDataBase::susRepChg(uint32_t dig, void (* fnChgReport)(digInput * dig)){
	digInputs[dig].userChgReport = fnChgReport;
}

int digDataBase::getDigState(uint32_t bit){
	return digInputs[bit].state;
}


/***************************************************** Class digInput **********************************************
 *
class digOutput {
public:
	volatile typeStateOutput state;
	volatile uint32_t valueField;
	volatile uint32_t function;
	volatile uint32_t ticks;
	volatile uint32_t ticksCount;
	volatile uint32_t port;
	volatile uint32_t pin;
	volatile uint32_t bit;
	volatile bool repChg;
	volatile typeAeiTime chgTime;
	void (* userChgReport)(digOutput * dig);

	digOutput();
	void construct(uint32_t _bit, uint32_t _port, uint32_t _pin, uint32_t time);
	void control(typeOutFunction fun);
	void update(typeOutFunction function);
};
*/

digOutput::digOutput(){
	state=outStOFF;
	valueField=NOT_UPDATED;
	function=outNoFunction;
	ticks=0;
	repChg=FALSE;
	userChgReport=0;
}

void digOutput::construct(uint32_t _bit, uint32_t _port, uint32_t _pin){
	bit=_bit;
	port=_port;
	pin=_pin;
	if(port == 1){
		P1[pin].function(PinIO::PIN_FN_OUT);
		P1[pin]=0;
	}
	else{
		P2[pin].function(PinIO::PIN_FN_OUT);
		P2[pin]=0;
	}
}

void digOutput::control(typeOutFunction fun, uint32_t time){
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

void digOutput::update(){
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
/*	if(port==1)
		P1[pin]=newValue;
	else
		P2[pin]=newValue;
*/
	if(port==1){
		if(newValue==0)	P1[pin]=0; else P1[pin]=1;}
	else{
		if(newValue==0)	P2[pin].clr(); else P2[pin].set();}

}

/***************************************************** Class digInput **********************************************
 *
class digInput {
public:
	volatile uint32_t stateField;
	volatile uint32_t stateLast;
	volatile uint32_t state;
	volatile uint32_t filterTicks;
	volatile uint32_t port;
	volatile uint32_t pin;
	volatile uint32_t bit;
	volatile bool repChg;
	volatile char alias[10];
	volatile typeAeiTime chgTime;
	void (* userChgReport)(digInput * dig);

	digInput();
	void creator(uint32_t _bit, uint32_t _port, uint32_t _pin);
	void update();
};
 *
 */

digInput::digInput(){
	stateField=NOT_UPDATED;
	stateLast=NOT_UPDATED;
	state=NOT_UPDATED;
	filterTicks=0;
	repChg=FALSE;
	userChgReport=0;
}

void digInput::construct(uint32_t _bit, uint32_t _port, uint32_t _pin){
	bit=_bit;
	port=_port;
	pin=_pin;
	if(port == 1){
		P1[pin].function(PinIO::PIN_FN_IN);
	}
	else{
		P2[pin].function(PinIO::PIN_FN_IN);
	}
}

void digInput::update(){
	stateLast = stateField;
	//Update from field
	if(port == 1)
		stateField = (uint32_t)P1[pin].readBack();
	else
		stateField = (uint32_t)P2[pin].readBack();

	//Filter
	if(stateLast == stateField){
		if(++filterTicks == (uint32_t)FILTER_TICKS){
			if(state != stateField){
				state = stateField;
				chgTime.sysTime = time(0);
				chgTime.msCurrSec = getMsTime();
				if(userChgReport!=0)
					userChgReport(this);
				if(repChg){
					askForService((int)repDigChg,(void *)this, FALSE);
				}
			}
		}
		else if(filterTicks>(uint32_t)FILTER_TICKS){
			filterTicks=(uint32_t)FILTER_TICKS;
		}
	}
	else{
		filterTicks=0;
	}
}



/*************************************** SET FUNCTIONS TO ATTEND ALL PROCESS **********************************/
void setFnsAttnProcess(uint32_t digNumber, typeProcess process, void (* fn)(digInput * dig)){
	switch(process){
		case procLoopDet:
			break;
		case procAxleDet:
			break;
		case procIoUtilities:
			break;
	}
	digDB.susRepChg(digNumber, fn);
}


