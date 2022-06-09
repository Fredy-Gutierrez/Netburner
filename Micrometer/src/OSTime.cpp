/*
 * OSTime.cpp
 *
 *  Created on: 21 sep 2021
 *      Author: Integra Fredy
 */
#include "headers/OSTime.h"
#include "headers/Business.h"
#include <stdlib.h>

volatile unsigned long long TestSecs = 0;
volatile uint32_t msCurrSec = 0;		//milisecs of current second
volatile uint32_t usCurrSec = 0;		//microsecs of current second, to get current value, multiplie by 200
volatile TimeStatus timeStatus = initStatus;

void updateUsec(){
	switch(timeStatus){
		case initStatus:
			TestSecs = 0;
			msCurrSec = 0;
			usCurrSec = 0;
			break;
		case gettingTimeStatus:
			if(usCurrSec == 5){
				msCurrSec++;
				usCurrSec = 0;
			}else{
				usCurrSec++;
			}
			if(msCurrSec == 1000){
				TestSecs++;
				msCurrSec = 0;
			}
			break;
		case stopStatus:
			timeStatus = initStatus;
			break;
		default:
			break;
	}
	attnWaitTime();
}

void getIOTimeDiference(IOTime * timeInit, IOTime * timeEnd, IOTime * destination){
	IOTime newTime = {0,0,0};
	unsigned long long init = (timeInit->uSec/200)+(timeInit->msSec*5)+(timeInit->Secs*5000);
	unsigned long long end = (timeEnd->uSec/200)+(timeEnd->msSec*5)+(timeEnd->Secs*5000);
	unsigned long long dif;
	if(end > init){
		dif = end - init;
	}else{
		dif = init - end;
	}
	if(dif>=5000){
		int secDif = dif / 5000;
		newTime.Secs = secDif;
		dif = dif - (5000 * secDif);
	}
	if(dif >= 5){
		int msDif = dif / 5;
		newTime.msSec = msDif;
		dif = dif - (5 * msDif);
	}
	newTime.uSec = dif * 200;
	destination->Secs = newTime.Secs;
	destination->msSec = newTime.msSec;
	destination->uSec = newTime.uSec;
}
void getElapseTime(volatile IOTime * t){
	t->Secs = TestSecs;
	if(usCurrSec == 5){
		t->msSec = msCurrSec+1;
		t->uSec = 0;
	}else{
		t->uSec = usCurrSec*200;
	}
	if(msCurrSec >= 1000){
		t->Secs = TestSecs + 1;
		t->msSec = 0;
	}else{
		t->msSec = msCurrSec;
	}
}
void stopGettingTime(){
	TestSecs = 0;
	msCurrSec = 0;
	usCurrSec = 0;
	timeStatus = stopStatus;
}
void startGettingTime(){
	timeStatus = gettingTimeStatus;
}
void initTimer(){
	timeStatus = initStatus;
}
