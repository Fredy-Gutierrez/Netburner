/*
 * SpeedProcess.cpp
 *
 *  Created on: 16 april 2021
 *      Author: Integra MGZ
 */


#include <basictypes.h>
#include "libs/Log.h"
#include "libs/Rtc_Controller.h"
#include "libs/IO_Interface.h"
#include "libs/AxleDetProcess.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"
#include "libs/Fifo/Fifo.h"

/***************************************************** Class speedProc **********************************************
*
class axleDetProc {
public:
	uint32_t format;
	int dir;
	uint32_t state;
	uint32_t digSys1;
	uint32_t digSys2;
	typeAeiTime onSys1;
	typeAeiTime onSys2;
	uint32_t difTime;
	int aeiDir;

	axleDetProc();
	void construct(uint32_t _digSys1, uint32_t _digSys2, int _aeiDir, uint32_t _format);
	void update(digInput * dig);
	void calcTime();

};
*/



axleDetProc::axleDetProc(){ // @suppress("Class members should be properly initialized")
	state = 0;
	digSys1 = 0xFF;
	digSys2 = 0xFF;
	difTime = 0;
	aeiDir = 0;
	dir = 0;
}

void axleDetProc::construct(uint32_t _digSys1, uint32_t _digSys2, int _aeiDir, uint32_t _format){
	digSys1=_digSys1;
	digSys2=_digSys2;
	aeiDir=_aeiDir;
	format = _format;
}

void axleDetProc::update(digInput * dig){

	if(digSys1==dig->bit || digSys2==dig->bit){
		if(dig->state==0){
			state=stIni;
		}
		else{
			switch(state){
				case stIni:
					if(digSys1==dig->bit){
						onSys1.sysTime=dig->chgTime.sysTime;
						onSys1.msCurrSec=dig->chgTime.msCurrSec;
						state=stWaitSys2;
					}
					if(digSys2==dig->bit){
						onSys2.sysTime=dig->chgTime.sysTime;
						onSys2.msCurrSec=dig->chgTime.msCurrSec;
						state=stWaitSys1;
					}
					break;
				case stWaitSys1:
					if(digSys1==dig->bit){
						onSys1.sysTime=dig->chgTime.sysTime;
						onSys1.msCurrSec=dig->chgTime.msCurrSec;
						calcTime();
					}
					state=stIni;
					break;
				case stWaitSys2:
					if(digSys2==dig->bit){
						onSys2.sysTime=dig->chgTime.sysTime;
						onSys2.msCurrSec=dig->chgTime.msCurrSec;
						calcTime();
					}
					state=stIni;
					break;
			}
		}
	}
}

void axleDetProc::calcTime(){
	info report;
	int32_t milisecs;

	milisecs = ( (int32_t)onSys2.sysTime-(int32_t)onSys1.sysTime)*1000 + ((int32_t)onSys2.msCurrSec- (int32_t)onSys1.msCurrSec);
	if(milisecs < 0){
		milisecs*=-1;
		dir=-1;
	}
	else{
		dir=1;
	}
	if(aeiDir==-1) dir*=-1;

	report.par1=(uint32_t)3600/(uint32_t)milisecs; //speed is stored as deca meters by hour
	report.par2=(uint32_t)dir;
	report.chgTime.sysTime=time(0);
	report.chgTime.msCurrSec=getMsTime();
	askForService(format, (void *)(&report), TRUE);
}
