#include <predef.h>
#include <stdio.h>
#include <nbrtos.h>
#include <http.h>
#include <init.h>
#include "libs/Business.h"
#include "libs/Default_Values.h"
#include "libs/Watch_Dog.h"
#include "libs/Recovery_Mode.h"
#include <iosys.h>

const char * AppName="APIVER03";

void UserMain(void * pd)
{
	init();
	StartHttp();
	WaitForActiveNetwork(TICKS_PER_SECOND * 5);
	checkDebugUart();//check if the uart1 is like the debug port
	initRecovery();//check if there is a jumper in rx and tx in uart0 (if there is a jumper, the system will reboot with the default values)*/
    initWatchDog();//start the watchdog

    iprintf("Application %s version %s started\n", AppName, APPVERSION);
/*********************************************NOTHING BEFORE***********************************************/
    initBusiness();

    while (1)
    {
        resetBitWatchDog();//reset the watchdog's bit
    	OSTimeDly(TICKS_PER_SECOND);
    }
}
