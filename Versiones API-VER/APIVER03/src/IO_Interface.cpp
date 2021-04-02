/*
 * IO_Interface.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#include "libs/IO_Interface.h"
#include "libs/Sargas_Connection.h"
#include <stdio.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <stopwatch.h>
#include <pins.h>
#include <pin_irq.h>
#include <iostream>
#include <HiResDelay.h>
#include <same70.h>
#include <core_cm7.h>
#include "FatLib/FileSystemUtils.h"


char * pFileName = "Interruptions.txt";

extern volatile uint32_t TimeTick;
extern volatile uint32_t CPU_CLOCK;


/**********************************<TIMER TICK>********************************/
/*static unsigned long long GetNow()
{
	uint32_t iv = SysTick->VAL;
	uint32_t tt = TimeTick;
	uint32_t ev = SysTick->VAL;

	if ((iv < ev) && (tt == TimeTick)) //TimTick incremented betweeniv and lv
	{
		tt--;
	}
	uint32_t lv = SysTick->LOAD;
	unsigned long long cv = tt;
	cv *= lv;
	cv += (lv - iv);
	return cv;
}*/

unsigned long long GetNow()
{
    uint32_t iv = SysTick->VAL;
    uint32_t tt = TimeTick;
    uint32_t ev = SysTick->VAL;
/* its possible TimeTick incremented between iv and ev
   we need to detect this and correct the captured value
   since systick counts down.... if iv> ev we had a rollover
   between the reads.... so did we capture the timetick
   value before or after the roll over?
   we know that if iv>ev the current TimeTick goes with ev...
   so we check... if captured tt == current TimeTick we
   go the wrong tt and need to adjust.
*/
    if ((iv < ev) && (tt == TimeTick))
    {
        tt--; //TimeTick incremented and we grabbed the wrong one
    }
    uint32_t lv = SysTick->LOAD;

    /*So for there to be a pending ISR we have to be calling this
	from withn a blocking ISR.
	//So if we have pending SYSTICK and ev shows that we have
	reloaded systick counter then
	//out captured timetick is one too low..*/

    if((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk) && (ev>(lv/2))) tt++;

    unsigned long long cv = tt;
    cv *= (lv+1);
    cv += (lv - iv);
    return cv;
}




//Convert a time or interval in unsigned long long to double/real.
double Convert(unsigned long long uv)
{
    double dv = uv;
    return dv / CPU_CLOCK;
}



/**********************************</TIMER TICK>********************************/

/**********************************<IRQ>********************************/
#define IRQ_POLARITY_FALLING   -1 //just when the input is 1 or >0
#define IRQ_POLARITY_EITHER     0 //for 0 and 1

volatile bool interruptionState = false;
PinIO pioPort = P2[6];
volatile int inputNum = 0;

/*struct inputs{
	bool state;
	unsigned long long tick;
};
volatile struct inputs digital_inputs[100000];*/

volatile bool firstInt = false;
volatile unsigned long long beforeTickTime = 0;
volatile bool beforeState;

const unsigned int filtermin = 59882;
const unsigned int filtermax = 62306;

typedef struct{
	bool state;
	unsigned long long difftick;
}wrongInputs;

wrongInputs volatile inputs[100000];


//1,212
//61,094
/*****************************************SYS2 INTERRUPTION******************************/
void extInt(int port, int portPin)
{
	unsigned long long actualTickTime = GetNow();
	if(interruptionState){
		bool state = pioPort.readBack();
		if(firstInt){
			if(inputNum < 100000){
				//if(state != beforeState){
					volatile unsigned long long tickDiff = actualTickTime - beforeTickTime;
					if(tickDiff > filtermax || tickDiff < filtermin){
						inputs[inputNum].state = state;
						inputs[inputNum].difftick = tickDiff;
						inputNum++;
					}
				//}
				/*unsigned long long tickDiff = actualTickTime - beforeTickTime;
				inputs[inputNum].state = state;
				inputs[inputNum].difftick = tickDiff;
				inputNum++;*/
			}


		}else{
			firstInt = true;
		}
		beforeTickTime = actualTickTime;
		beforeState = state;
	}
}


void getInputDiff(){
	if(interruptionState){
		DisableIrq(pioPort);
		interruptionState = false;
	}

	f_enterFS();

	InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive

	DeleteFile(pFileName);

	double time;

#define WRITE_BUFSIZE 80

	/*for (int x = 0; x < inputNum; ++x) {
		time = 0;

		if(x > 0){
			time = Convert(digital_inputs[x].tick - digital_inputs[x-1].tick);
		}



		char write_buf[WRITE_BUFSIZE];
		sniprintf(write_buf, WRITE_BUFSIZE, "Input value: %d Time diff: %g sec\n", digital_inputs[x].state, time);

		AppendFile((unsigned char *)write_buf, pFileName, strlen(write_buf));

	}*/

	for (int x = 0; x < inputNum; x++) {
		/*time = 0;
		if(x > 0){*/
		time = Convert(inputs[x].difftick);
		//}
		char write_buf[WRITE_BUFSIZE];
		sniprintf(write_buf, WRITE_BUFSIZE, "Input value: %d Time diff: %g sec Number Tick: %llu\n", inputs[x].state, time, inputs[x].difftick);

		AppendFile((unsigned char *)write_buf, pFileName, strlen(write_buf));

	}

	UnmountExtFlash();

	f_releaseFS();

	iprintf("LOAD VAL: %lu\n", SysTick->LOAD);
}


void startInterruption(){
	if(!interruptionState){
		memset((void *)inputs, 0, sizeof(inputs));
		inputNum = 0;
		EnableIrq(pioPort);
		interruptionState = true;
		firstInt = false;
	}
}


/*****************************************CONFIG IO AND IRQ´S******************************/
void configureIO(){
/***************************************PINS NUMS FOR INPUTS********************************/

	SetPinIrq( pioPort, IRQ_POLARITY_EITHER, extInt);
	DisableIrq(pioPort);
	interruptionState = false;
}


/*****************************************INIT THE CONFIGURATIONS******************************/
void initIO(){
	configureIO();
}



