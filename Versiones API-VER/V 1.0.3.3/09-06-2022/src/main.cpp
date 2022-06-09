#include <crypto/ssl.h>
#include <predef.h>
#include <stdio.h>
#include <nbrtos.h>
#include <http.h>
#include <init.h>
#include <iosys.h>
#include <stdlib.h>
#include <basictypes.h>
#include "libs/Default_Values.h"
#include "libs/Watch_Dog.h"
#include "libs/Recovery_Mode.h"
#include "libs/IO_Interface.h"
#include "libs/SargasProcess.h"
#include "libs/Log.h"
#include "libs/Business.h"
#include "libs/I2C_Controller.h"
#include "libs/Configurations.h"
#include <HiResDelay.h>
#include <libs/TagProcessor.h>

#include <pins.h>

const char * AppName="MAC30\0"; //Mini AEI Controller 30(anniversary of Integra)
bool restartByWatchDog;

void UserMain(void * pd){
	init();
	StartHttp();
	WaitForActiveNetwork(TICKS_PER_SECOND * 5);

    initWatchDog();//start the watchdog

	OSTimeDly(TICKS_PER_SECOND * 3);//TIME TO WAIT THE FUNCTIONS OF NETBURNER START
	//checkDebugUart();//check if the uart1 is like the debug port
	initRecovery();//check if there is a jumper in rx and tx in uart0 (if there is a jumper, the system will reboot with the default values)
	iprintf("\nNo recovery mode. Using configuration from flash memory, for AEI: %s", getAeiID());

	if( getQtySargas() == 1 ){
		iprintf("\n\n\nBooting ....... %s app ver. %s,  AEI %s,  IP Add. %s,  (%d)Sargas with IP Add. (R,%d)=%s, LoopType %s", AppName, APPVERSION, getAeiID(), getNbIpAddress(), getQtySargas(), getQtySargasAnt('R'), getSargasIp('R'), getLoopTypeStr() );
	}
	else{
		iprintf("\n\n\nBooting ....... %s app ver. %s,  AEI %s,  IP Add. %s,  (%d)Sargas with IP Add. (R,%d)=%s, (L,%d)=%s, LoopType %s", AppName, APPVERSION, getAeiID(), getNbIpAddress(), getQtySargas(), getQtySargasAnt('R'), getSargasIp('R'), getQtySargasAnt('L'), getSargasIp('L'), getLoopTypeStr() );
	}
	iprintf("\nLoop type %s", getLoopTypeStr());
	OSTimeDly(TICKS_PER_SECOND);

    initProcessor();
	initSargas();

	initI2C_Controller();

	initIO();
	initBusiness();
	/*sprintf(strPrn, "\nApplication %s version %s started", AppName, APPVERSION);
	logg.prn(strPrn);*/
	logg.newPrint("\nApplication %s version %s started", AppName, APPVERSION);
	restartByWatchDog=FALSE;

	//DelayObject delay;
	int ticks = 0;
    while(1){
    	printLogs();
		logg.printNextMsg();
    	if(++ticks >= TICKS_PER_SECOND){
			if(!restartByWatchDog)
				resetBitWatchDog();//reset the watchdog's bit
			attendBusiness();
			ticks = 0;
    	}
    	OSTimeDly(1);
    	//delay.DelayUsec(1000);
    }
}
