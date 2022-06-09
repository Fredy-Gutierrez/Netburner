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

/*
typedef struct{
	int size;
	char data[50];
}d;
d datum[100];
d datum1[100];
F_FILE *fp1;


void saveData(d dat[], int qty){
	long qtyItems;
	for(int i=0; i<qty; i++){
		qtyItems=f_write((void *) & dat[i], sizeof(d), 1, fp1);
		if(qtyItems!=1){
			iprintf("\nError writing line %d, written %d items", i, (int)qtyItems);
		}
	}
}

void retrieveData(d dat[], int qty){
	long qtyItems;
	for(int i=0; i<qty; i++){
		qtyItems=f_read((void *) & dat[i], sizeof(d), 1, fp1);
		if(qtyItems!=1){
			iprintf("\nError reading %d, read %d items", i, (int)qtyItems);
		}
	}
}


void fillData(d dat[], int qty){
	for(int i=0; i<qty; i++){
		dat[i].size=10+i;
		if(dat[i].size>50)
			dat[i].size=50;
		int j;
		for(j=0; j<dat[i].size; j++){
			dat[i].data[j]=j;
		}
		for(; j<50; j++){
			dat[i].data[j]=0x55;
		}
	}
}

void printData(d dat[], int qty){
	for(int i=0; i<qty; i++){
		iprintf("\nLine %d, size %d: ", i, dat[i].size);
		for(int j=0; j<50; j++){
			iprintf("%d,",dat[i].data[j]);
		}
	}
}

void UserMain(void * pd){
	init();
	StartHttp();
	WaitForActiveNetwork(TICKS_PER_SECOND * 5);

	int rv=f_enterFS();
	if(rv==0){
		rv=initSDCard();
		if(rv==0){
			fp1=openFile("Data.bin", "w+");
			if(fp1!=NULL){
				fillData(datum, 50);
				printData(datum, 50);
				saveData(datum, 50);
				f_close(fp1);
			}
		}
	}
	if(rv!=0)
		iprintf("\nError numero %d al inicializar la memoria SD", rv);
	else{
		fp1=openFile("Data.bin", "r");
		if(fp1!=NULL){
			retrieveData(datum1, 50);
			f_close(fp1);
			printData(datum1, 50);
		}

	}
}
*/
