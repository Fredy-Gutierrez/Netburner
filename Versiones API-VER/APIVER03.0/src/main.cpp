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

const char * AppName = "APIVER03";

void UserMain(void * pd) {
	init();
	StartHttp();
	WaitForActiveNetwork(TICKS_PER_SECOND * 5);
	checkDebugUart();
	initRecovery();
	initWatchDog();

	iprintf("Application %s version %s started\n", AppName, APPVERSION);
	/*********************************************NOTHING BEFORE***********************************************/
	initBusiness();

	while (1) {
		serviceWatchdog();
		OSTimeDly(TICKS_PER_SECOND);
	}
}
