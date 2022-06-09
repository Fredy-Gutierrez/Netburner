/*
 * LoopProcess.cpp
 *
 *  Created on: 16 april 2021
 *      Author: Integra MGZ
 */


#include <basictypes.h>
#include "libs/log.h"
#include "libs/Rtc_Controller.h"
#include "libs/IO_Interface.h"
#include "libs/LoopProcess.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"
#include "libs/Fifo/Fifo.h"



/***************************************************** Class loopDet **********************************************
*
	class loopDet {
		loopState state;
		loopType type;
		uint32_t digInC;
		uint32_t digInI;
		uint32_t digInO;
		uint32_t fieldStInI;
		uint32_t fieldStInO;
		//uint32_t maxSecsOn;
		uint32_t minSecsOff;
		//bool flagTON;
		bool flagTOFF;
		typeAeiTime timeChg;
		//time_t TON;
		time_t TOFF;

		loopDet();
		void construct(uint32_t _digInputC_I, uint32_t _digInputO, loopType _type, uint32_t _minSecsOff);
		void updateType(loopType _type);
		void update(digInput * dig);
		void timer();
	};
*/

loopDet::loopDet(){ // @suppress("Class members should be properly initialized")
	state=stInactive;
	fieldStInI=0;
	fieldStInO=0;
	//flagTON=FALSE;
	flagTOFF=FALSE;
	type = loopSingle;
	//maxSecsOn=300;
	minSecsOff=120;
}

void loopDet::construct(uint32_t _digInputC_I, uint32_t _digInputO, loopType _type, uint32_t _minSecsOff){
	type = _type;
	//maxSecsOn=_maxSecsOn;
	minSecsOff=_minSecsOff;
	if(type==loopSingle){
		digInC=_digInputC_I;
		digInI=0xFF;
		digInO=0xFF;
	}
	else {
		digInC=0xFF;
		digInI=_digInputC_I;
		digInO=_digInputO;
	}
}
void loopDet::updateType(loopType _type){
	type=_type;
}

void loopDet::update(digInput * dig){
	uint32_t st;
	void * pt=NULL;

	if(type==loopSingle){
		if(digInC!=dig->bit)
			return;
		st=dig->state;
	}
	else {
		if(digInI==dig->bit)
			fieldStInI=dig->state;
		else if(digInO==dig->bit)
			fieldStInO=dig->state;
		else
			return;
		st=fieldStInI|fieldStInO;
	}

	switch(state){
	case stInactive:
		if(st==1){
			//Turn on antennas
			askForService(turnOnAnt, pt, TRUE);
			//TON=dig->chgTime.sysTime;
			//flagTON=TRUE;
			flagTOFF=FALSE;
			//Start car detected
			//askForService(infoStartCarDet);
			state=stActive;
		}
		break;

	case stActive:
		if(st==0){
			TOFF=dig->chgTime.sysTime;
			flagTOFF=TRUE;
			//flagTON=FALSE;
			//End car detected
			//askForService(infoEndCarDet);
			state=stEndCarDet;
		}
		break;

	case stEndCarDet:
		if(st==1){
			//TON=dig->chgTime.sysTime;
			//flagTON=TRUE;
			flagTOFF=FALSE;
			//Start car detected
			//askForService(infoStartCarDet);
			state=stActive;
		}
		break;
	}
}

void loopDet::timer(){
	void * pt=NULL;
	if(flagTOFF){
		if((time(0)-TOFF)>minSecsOff){
			switch(state){
				case stInactive:
					break;
				case stActive:
					break;
				case stEndCarDet:
					//Apaga antenas
					askForService(turnOffAnt, pt, TRUE);
					state=stInactive;
					break;
			}
		}

	}
}
/*
void loopDet::update(digInput * dig){
	uint32_t st;
	void * pt=NULL;

	if(type==loopSingle){
		if(digInC!=dig->bit)
			return;
		st=dig->state;
	}
	else {
		if(digInI==dig->bit)
			fieldStInI=dig->state;
		else if(digInO==dig->bit)
			fieldStInO=dig->state;
		else
			return;
		st=fieldStInI|fieldStInO;
	}

	switch(state){
	case stInactive:
		if(st==1){
			state=stActive;
			//Turn on antennas
			askForService(turnOnAnt, pt, TRUE);
			TON=dig->chgTime.sysTime;
			flagTON=TRUE;
			flagTOFF=FALSE;
			//Start car detected
			//askForService(infoStartCarDet);
		}
		break;

	case stActive:
		if(st==0){
			state=stEndCarDet;
			TOFF=dig->chgTime.sysTime;
			flagTOFF=TRUE;
			flagTON=FALSE;
			//End car detected
			//askForService(infoEndCarDet);
		}
		break;

	case stEndCarDet:
		if(st==1){
			state=stActive;
			TON=dig->chgTime.sysTime;
			flagTON=TRUE;
			flagTOFF=FALSE;
			//Start car detected
			//askForService(infoStartCarDet);
		}
		break;
	}
}

void loopDet::timer(){
	void * pt=NULL;
	if(flagTON){
		if((time(0)-TON)>maxSecsOn){
			switch(state){
				case stInactive:
					break;
				case stActive:
					state=stInactive;
					//Apaga antenas
					askForService(turnOffAnt, pt, TRUE);
					//CarStopOver
					//askForService(infoCarStopOver);
					break;
				case stEndCarDet:
					break;
			}
		}
	}
	else if(flagTOFF){
		if((time(0)-TOFF)>minSecsOff){
			switch(state){
				case stInactive:
					break;
				case stActive:
					break;
				case stEndCarDet:
					state=stInactive;
					//Apaga antenas
					askForService(turnOffAnt, pt, TRUE);
					break;
			}
		}

	}
}
*/
