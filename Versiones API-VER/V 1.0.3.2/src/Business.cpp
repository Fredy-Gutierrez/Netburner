/*
 * Business.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 *
 */
#include <arp.h>
#include <libs/TagProcessor.h>
#include <libs/TagUtilities.h>
#include <netinterface.h>
#include <stdlib.h>
#include <stdio.h>
#include <pins.h>
#include <nettypes.h>
#include <string.h>
#include "libs/Log.h"
#include "libs/Default_Values.h"
#include "libs/Tcp_Class.h"
#include "libs/I2C_Controller.h"
#include "libs/Rtc_Controller.h"
#include "libs/IO_Interface.h"
#include "libs/Business.h"
#include "libs/SargasProcess.h"
#include "libs/AxleDetProcess.h"
#include "libs/LoopProcess.h"
#include "libs/fifo/Fifo.h"
#include "libs/Report2Server.h"
#include "libs/Configurations.h"
#include "libs/AxleProcess.h"
#include "libs/Train.h"
#include "libs/DBTrainDriver.h"

uint32_t SargasTcpTaskStack[USER_TASK_STK_SIZE];

extern bool restartByWatchDog; // @suppress("Unused variable declaration in file scope")

logger logg;
typedef struct{
	bool started;
	bool scanDigs;
	bool prnDigChgs;
	bool prnLoopDigChgs;
	bool prnTagMsgs;
	bool prnAxlMsgs;
	loopType loop;
	bool synch;
	char synchTime[24];
} typeStatusBus;
typeStatusBus statusBus;

FifoInfo infList;
FifoCmd cmdList;
loopDet loop;
axleDetProc mainAxleDet;
axleDetProc secAxleDet;
axleDetProc mirrorAxleDet;
speed speedDmh;
axleProc axleProcess;
clsStatus stMac30;
clsBattery battery;

bool testMode = false;

void startIoUtilities();
void startLoop();
void attnLoop(digInput * dig);
void startAxleDets();
void attnMainAxleDet(digInput * dig);
void attnSecAxleDet(digInput * dig);
void attnMirrorAxleDet(digInput * dig);
void initSargas();
void turnRfOnSargas(int numSargas);
void turnRfOffSargas(int numSargas);
void startSargas(int numSargas);
void stopSargas(int numSargas);
void updateAcPowerStatus();
void updateDcPowerStatus();

void updateSychStatus(bool _synch){
	statusBus.synch=_synch;
	if(_synch){
		getNowTimeAscii(statusBus.synchTime);
		statusBus.synchTime[20]='\0';
	}
}

bool getSychStatus(){
	return statusBus.synch;
}

/*****************************************MENU FOR THE TERMINAL (SERIAL PORT)******************************/
void displayMenu(){

	if(getQtySargas()==1){
		logg.newPrint("\n\n***  %s ver. %s,  AEI %s,  IP Add. %s,  (%d)Sargas with IP Add. (R,%d)=%s, LoopType %s", AppName, APPVERSION, getAeiID(), getNbIpAddress(), getQtySargas(), getQtySargasAnt('R'), getSargasIp('R'), getLoopTypeStr() );
	}
	else{
		logg.newPrint("\n\n***  %s ver. %s,  AEI %s,  IP Add. %s,  (%d)Sargas with IP Add. (R,%d)=%s, (L,%d)=%s, LoopType %s", AppName, APPVERSION, getAeiID(), getNbIpAddress(), getQtySargas(), getQtySargasAnt('R'), getSargasIp('R'), getQtySargasAnt('L'), getSargasIp('L'), getLoopTypeStr() );
	}
    if(sargasR.getReading()){
    	logg.newPrint("\nSargas antenna ON\n");}
    else{
    	logg.newPrint("\nSargas antenna OFF\n");}
    logg.newPrint("[D] Display digital values\n");
    logg.newPrint("[H] Display current system time\n");
    if(statusBus.scanDigs)
    	logg.newPrint("[S] Toggle scan of digitals [Started]\n");
    else
    	logg.newPrint("[S] Toggle scan of digitals [Stopped]\n");
    if(sargasR.getProc())
    	logg.newPrint("[R] Toggle Sargas reader process [Started]\n");
    else
    	logg.newPrint("[R] Toggle Sargas reader process [Stopped]\n");
    if(statusBus.prnDigChgs)
    	logg.newPrint("[P] Toggle printing of digital changes [Enabled]\n");
    else
    	logg.newPrint("[P] Toggle printing of digital changes [Disabled]\n");
    if(statusBus.prnLoopDigChgs)
    	logg.newPrint("[X] Toggle printing of loop digital input changes [Enabled]\n");
    else
    	logg.newPrint("[X] Toggle printing of loop digital input changes [Disabled]\n");
    if(statusBus.loop==loopSingle)
    	logg.newPrint("[L] Change loop type [Single]\n");
    else if(statusBus.loop==loopDouble)
    	logg.newPrint("[L] Change loop type [Double]\n");
    else
    	logg.newPrint("[L] Change loop type [Redundant]\n");
    if(logg.getMsgPrn())
    	logg.newPrint("[M] Toggle printing Sargas messages [Enabled]\n");
    else
    	logg.newPrint("[M] Toggle printing Sargas messages [Disabled]\n");
    if(statusBus.prnAxlMsgs)
    	logg.newPrint("[Q] Toggle printing axle messages [Enabled]\n");
    else
    	logg.newPrint("[Q] Toggle printing axle messages [Disabled]\n");
    logg.newPrint("[B] Make battery test\n");
    logg.newPrint("[T] Print ticks duration\n");
    logg.newPrint("[U] Print status sent to server\n");
    logg.newPrint("[Y] Print synch status\n");
    logg.newPrint("[+] Turn ON RFID antennas\n");
    logg.newPrint("[-] Turn OFF RFID antennas\n");
    logg.newPrint("[1] Set output 1\n");
    logg.newPrint("[2] Reset output 1\n");
    logg.newPrint("[3] Set output 2\n");
    logg.newPrint("[4] Reset output 2\n");

    if(testMode){
    	logg.newPrint("[5] Test Mode ACTIVE (THE PROFILER IS NOT WORKING)\n");
    }else{
    	logg.newPrint("[5] Test Mode INACTIVE (THE PROFILER IS WORKING)\n");
    }

    logg.newPrint("[N] Display quantity of trains not sent\n");
    logg.newPrint("[&] Force reboot by watch-dog, can not be reverted \n");
    logg.newPrint("[*] Force reboot, instantly\n");
    logg.newPrint("\n");
}

void printSynch(){
	if(statusBus.synch){
		logg.newPrint("\nSystem is synchronized, last sych time = %s", statusBus.synchTime);
	}
	else{
		logg.newPrint("\nSystem is NOT synchronized, last sych time = %s", statusBus.synchTime);
	}
	sargasR.getDateTime();
}

void printStatus(){
	logg.newPrint("\n%s", stMac30.getStatus());
}


/***************************************** TERMINAL COMMAND ******************************/
void terminal(void *pd){
	int rep=0;
	while(true){
		char c = getchar();
		iprintf("\n\n");
		switch (c){
			case 'D': case'd':
				printDigitals();
				rep=0;
				break;
			case 'H': case 'h':
				printSystemTime();
				rep=0;
				break;
			case 'S': case's':
			    if(statusBus.scanDigs){
			    	logg.newPrint("\nStop of scan of digital inputs by user request");
					ioStop();
					statusBus.scanDigs=FALSE;
			    }
			    else{
			    	logg.newPrint("\nStart of scan of digital inputs by user request");
					ioStart();
					statusBus.scanDigs=TRUE;
			    }
				rep=0;
				break;
			case 'R': case'r':
			    if(sargasR.getProc()){
			    	logg.newPrint("\nStop of Sargas reader process by user request");
					stopSargas(sargasBoth);
			    }
			    else{
			    	logg.newPrint("\nStart of Sargas reader process by user request");
					startSargas(sargasBoth);
			    }
				rep=0;
				break;
			case 'P': case'p':
			    if(statusBus.prnDigChgs){
			    	logg.newPrint("\nDisable of printing of digital changes by user request");
					ioPrnChgDisable(ALL_DIGS);
					statusBus.prnDigChgs=FALSE;
			    }
			    else{
			    	logg.newPrint("\nEnable of printing of digital changes by user request");
					ioPrnChgEnable(ALL_DIGS);
					statusBus.prnDigChgs=TRUE;
			    }
				rep=0;
				break;
			case 'X': case'x':
			    if(statusBus.prnLoopDigChgs){
			    	logg.newPrint("\nDisable of printing of loop digital input changes by user request");
					ioPrnChgDisable(sgGetDigInputBit(idxLoopO));
					ioPrnChgDisable(sgGetDigInputBit(idxLoopC_I));
					statusBus.prnLoopDigChgs=FALSE;
			    }
			    else{
			    	logg.newPrint("\nEnable of printing of loop digital input changes by user request");
					ioPrnChgEnable(sgGetDigInputBit(idxLoopO));
					ioPrnChgEnable(sgGetDigInputBit(idxLoopC_I));
					statusBus.prnLoopDigChgs=TRUE;
			    }
				rep=0;
				break;
			case 'L': case'l':
			    if(statusBus.loop==loopSingle){
			    	logg.newPrint("\nChanging loop type to double by user request");
					statusBus.loop=loopDouble;
			    }
			    else{
			    	logg.newPrint("\nChanging loop type to single by user request");
					statusBus.loop=loopSingle;
			    }
				loop.updateType(statusBus.loop);
				rep=0;
				break;
			case 'M': case'm':
			    if(logg.getMsgPrn()){
			    	logg.newPrint("\nDisabling Sargas messages printing by user request");
					logg.setMsgPrn(FALSE);
			    }
			    else{
			    	logg.newPrint("\nEnabling Sargas messages printing by user request");
					logg.setMsgPrn(TRUE);
			    }
				loop.updateType(statusBus.loop);
				rep=0;
				break;
			case 'Q': case'q':
			    if(statusBus.prnAxlMsgs){
			    	logg.newPrint("\nDisable of printing axle messages by user request");
					statusBus.prnTagMsgs=FALSE;
					statusBus.prnAxlMsgs=FALSE;
			    }
			    else{
			    	logg.newPrint("\nEnable of printing axle messages by user request");
					statusBus.prnTagMsgs=TRUE;
					statusBus.prnAxlMsgs=TRUE;
			    }
				rep=0;
				break;
			case 'B': case'b':
				logg.newPrint("\nScheduling battery test in less than 1 minute, by user request");
				battery.testByUserRequest=TRUE;
				rep=0;
				break;
			case 'T': case't':
				printTicks();
				rep=0;
				break;
			case 'U': case'u':
				printStatus();
				rep=0;
				break;
			case 'Y': case'y':
				printSynch();
				rep=0;
				break;
			case '+':
				logg.newPrint("\nTurning ON RFID antennas by user request");
				turnRfOnSargas(sargasBoth);
				rep=0;
				break;
			case '-':
				logg.newPrint("\nTurning OFF RFID antennas by user request");
				turnRfOffSargas(sargasBoth);
				rep=0;
				break;
			case '1':
				logg.newPrint("\nControl output 1 to ON by user request");
				ioDigOutputControl(0, outLatchON, 0);
				rep=0;
				break;
			case '2':
				logg.newPrint("\nControl output 1 to OFF by user request");
				ioDigOutputControl(0, outLatchOFF, 0);
				rep=0;
				break;
			case '3':
				logg.newPrint("\nControl output 2 to OFF by user request");
				ioDigOutputControl(1, outLatchON, 0);
				rep=0;
				break;
			case '4':
				logg.newPrint("\nControl output 2 to OFF by user request");
				ioDigOutputControl(1, outLatchOFF, 0);
				rep=0;
				break;
			case '5':
				if(testMode){
					logg.newPrint("\nAEI in NORMAL OPERATION");
					testMode = false;
				}else{
					logg.newPrint("\nAEI in TEST MODE");
					testMode = true;
				}
				rep=0;
				break;
			case 'N':case 'n':
				logg.newPrint("\nTrains waiting to send: %d", (int)getQtyTrainsToSend());
				rep=0;
				break;
			case '&':
				logg.newPrint("\nForced reboot by user request, waiting for watch-dog");
				restartByWatchDog=TRUE;
				//restartMicroChip();
				rep=0;
				break;
			case '*':
				//logg.newPrint("\nForced reboot by user request, waiting for watchdog");
				//restartByWatchDog=TRUE;
				restartMicroChip();
				rep=0;
				break;
			default :
				if(++rep > 1){
					displayMenu();
					rep=0;
				}
				break;
		}

	}
}


void terminalCommand(){
	OSSimpleTaskCreatewName(terminal,TERMINAL_PRIO,"Terminal");
}

/***************************************** END OF TERMINAL COMMAND ******************************/

/***************************************** INTERNAL SERVICES ******************************/
void services(void *pd){
	info command;
	while(true){
		if(cmdList.Size()!=0){
			if(cmdList.Get(& command)==1){
				switch(command.format){
					case noService:
					default:
						break;
					case infoStartCarDet:
						break;
					case infoEndCarDet:
						break;
					case infoCarStopOver:
						break;
					case turnOnAnt:
						stMac30.setStatus(idxLoopDet, 'N', 32);
						turnRfOnSargas(sargasBoth);
						break;
					case turnOffAnt:
						stMac30.setStatus(idxLoopDet, 'N', 31);
						turnRfOffSargas(sargasBoth);
						break;
					case repMainAxle:
					case repSecAxle:
						speedDmh.updateSpeed(command.par1, (int)command.par2);
						axleProcess.add(& command);
						break;
					case repMirrorAxle:
						break;
				}
			}
		}
    	OSTimeDly(1);//Delay 50 milisec
	}
}

void askForService(uint32_t format, void * obj, bool saveCmd){
	infList.Add(format, obj);
	if(saveCmd)
		cmdList.Add(format, obj);
}
void internalServices(){
	OSSimpleTaskCreatewName(services,INT_SERVICES_PRIO,"Services");
}

/***************************************** END OF INTERNAL SERVICES ******************************/


void initAllProcess(){
	startSargas(sargasBoth);
	startIoUtilities();
	startLoop();
	startAxleDets();
	startProcessor();
	ioStart();
	initRtc();
	initRep2Server();
	InitDBTrain();
}


/*****************************************STARTS ALL THE SYSTEM'S PROCCESS******************************/
void initBusiness(){
	statusBus.started=FALSE;
	statusBus.prnDigChgs=FALSE;
	statusBus.prnLoopDigChgs=FALSE;
	statusBus.prnTagMsgs=TRUE;
	statusBus.prnAxlMsgs=TRUE;
	statusBus.scanDigs=TRUE;
	statusBus.loop=loopDouble;
	statusBus.synch=FALSE;
	getNowTimeAscii(statusBus.synchTime);
	statusBus.synchTime[20]='\0';
	initTrainAdmin();
	terminalCommand();
	internalServices();
	initAllProcess();
	stMac30.setStatus(idxMac30, 'N', 1);
	//logg.prn("\n\nSystem will NOT start if NTP time is not received and Sargas is not Synchronized by this application");
}

void attendBusiness(){
/*	if(!statusBus.started){
		if(statusBus.synch){
			initAllProcess();
			statusBus.started=TRUE;
		}
	}*/
	loop.timer();
	battery.timer();
}

/***************************************** FUNCTIONS TO ATTEND ALL PROCESS **********************************/
void printLogs(void){
	char asciiTime[30];
	info inf;
	//while(true){
		if(infList.Size() != 0){
			if(infList.Get( &inf ) == 1){
				convertTimeToASCII( &inf.chgTime, asciiTime );
				//sprintf(strPrn," ");
				switch(inf.format){
					case noService:
					default:
						//sprintf(strPrn, "\nNo service");
						break;
					case infoStartCarDet:
						//sprintf(strPrn, "\nStart of car detected");
						break;
					case infoEndCarDet:
						//sprintf(strPrn, "\nEnd of car detected");
						break;
					case infoCarStopOver:
						logg.newPrint("\nCar stop over");
						break;
					case turnOnAnt:
						logg.newPrint("\n%s RFID antenna turned ON by loop detector", asciiTime);
						break;
					case turnOffAnt:
						logg.newPrint("\n%s RFID antenna turned OFF by loop detector", asciiTime);
						break;
					case repDigChg:
						logg.newPrint("\n%s Digital change[%lu][%s]=%lu", asciiTime, inf.par1+1, ioGetDigInputId((int)inf.par1), inf.par2);
						break;
					case repMainAxle:
					case repSecAxle:
						if(statusBus.prnAxlMsgs){
							if((int)inf.par2 == -1)
								logg.newPrint("\n%s Axle with speed %lu, direction BACKWARD", asciiTime, inf.par1);
							else
								logg.newPrint("\n%s Axle with speed %lu, direction FORWARD", asciiTime, inf.par1);
						}
						break;
					case repMirrorAxle:
						if(statusBus.prnAxlMsgs){
							if((int)inf.par2 == -1)
								logg.newPrint("\n%s Mirror axle with speed %lu, direction BACKWARD", asciiTime, inf.par1);
							else
								logg.newPrint("\n%s Mirror axle with speed %lu, direction FORWARD", asciiTime, inf.par1);
						}
						break;
				}
			}
		}
	//}
}


/***************************************** LOOP PROCESS ****************************************************/
void startLoop(){
	loopType loopT=(loopType)getLoopType();
	loop.construct(sgGetDigInputBit(idxLoopC_I),sgGetDigInputBit(idxLoopO),loopT, getLoopTimeOff());
	setFnsAttnProcess(sgGetDigInputBit(idxLoopC_I), procLoopDet, attnLoop);
	setFnsAttnProcess(sgGetDigInputBit(idxLoopO), procLoopDet, attnLoop);
	stMac30.setStatus(idxLoopDet, 'N', 31);
}

void attnLoop(digInput * dig){
	loop.update(dig); //This function is called by digital when changed
}

/***************************************** END LOOP PPROCESS ****************************************************/

/***************************************** AXLE DET PROCESS ****************************************************/
void startAxleDets(){
	int aeiDir;
	aeiDir=getAeiDir();

	/*speedProcess.construct(2, 16, aeiDir);
	setFnsAttnProcess(2, procSpeed, attnSpeed);
	setFnsAttnProcess(16, procSpeed, attnSpeed);*/

	//This is done provisionaly because one of the digital inputs failed.
	mainAxleDet.construct(sgGetDigInputBit(getMainADSignalIndex()), sgGetDigInputBit(getMainADSignalIndex()+1), aeiDir, repMainAxle);
	setFnsAttnProcess(sgGetDigInputBit(getMainADSignalIndex()), procAxleDet, attnMainAxleDet);
	setFnsAttnProcess(sgGetDigInputBit(getMainADSignalIndex()+1), procAxleDet, attnMainAxleDet);
	stMac30.setStatus(idxAxleDet, 'N', 21);

	if(getDoubleAxDet()){
		logg.newPrint("\nInitializing second axle detector");
		secAxleDet.construct(sgGetDigInputBit(getDoubleADSignalIndex()), sgGetDigInputBit(getDoubleADSignalIndex()+1), getDirDoubleAxDet(), repSecAxle);
		setFnsAttnProcess(sgGetDigInputBit(getDoubleADSignalIndex()), procAxleDet, attnSecAxleDet);
		setFnsAttnProcess(sgGetDigInputBit(getDoubleADSignalIndex()+1), procAxleDet, attnSecAxleDet);
	}

	if(getMirrorAxDet()){
		logg.newPrint("\nInitializing mirror axle detector");
		mirrorAxleDet.construct( sgGetDigInputBit(getMirrorADSignalIndex()), sgGetDigInputBit(getMirrorADSignalIndex()+1), getDirMirrorAxDet(), repMirrorAxle );
		setFnsAttnProcess( sgGetDigInputBit(getMirrorADSignalIndex()), procAxleDet, attnMirrorAxleDet );
		setFnsAttnProcess( sgGetDigInputBit(getMirrorADSignalIndex()+1), procAxleDet, attnMirrorAxleDet );
	}

}
void attnMainAxleDet(digInput * dig){
	mainAxleDet.update(dig); 			//This function is called by digital when changed
}

void attnSecAxleDet(digInput * dig){
	secAxleDet.update(dig); 			//This function is called by digital when changed
}

void attnMirrorAxleDet(digInput * dig){
	mirrorAxleDet.update(dig); 			//This function is called by digital when changed
}

/***************************************** END SPEED PPROCESS ****************************************************/

/***************************************** SARGAS PROCESS ****************************************************/
void attnSargas(void * pd){
	while(1){
		sargasR.attend(); 			//This function is called every time interruption is called
		sargasL.attend(); 			//This function is called every time interruption is called
	}
}

void turnRfOnSargas(int numSargas){
	if(numSargas==sargasLeft){
		sargasL.rfOn();
	}
	else if(numSargas==sargasRight){
		sargasR.rfOn();
	}
	if(numSargas==sargasBoth){
		sargasR.rfOn();
		sargasL.rfOn();
	}
}

void turnRfOffSargas(int numSargas){
	if(numSargas==sargasLeft){
		sargasL.rfOff();
	}
	else if(numSargas==sargasRight){
		sargasR.rfOff();
	}
	if(numSargas==sargasBoth){
		sargasR.rfOff();
		sargasL.rfOff();
	}
}

void startSargas(int numSargas){
	if(numSargas==sargasLeft){
		sargasL.start();
	}
	else if(numSargas==sargasRight){
		sargasR.start();
	}
	if(numSargas==sargasBoth){
		sargasR.start();
		sargasL.start();
	}
}

void stopSargas(int numSargas){
	if(numSargas==sargasLeft){
		sargasL.stop();
	}
	else if(numSargas==sargasRight){
		sargasR.stop();
	}
	if(numSargas==sargasBoth){
		sargasR.stop();
		sargasL.stop();
	}
}

void initSargas(){
	sargasR.init('R', getQtySargasAnt('R'), TRUE);
	if(getQtySargas()==1){
		sargasL.noActive();
	}
	else{
		sargasL.init('L', getQtySargasAnt('L'));
	}
	OSTaskCreatewName(attnSargas, (void *) NULL, &SargasTcpTaskStack[USER_TASK_STK_SIZE],	SargasTcpTaskStack, SARGAS_PRIO, "Sargas");
}

/***************************************** END SARGAS PPROCESS ****************************************************/

/***************************************** BATTERY TEST PROCESS ****************************************************/

/*
class clsBattery {
public:
	int state;
	bool testOn;
	int tick;
	bool replace;
	bool testByUserRequest;

	clsBattery();
	void timer();
	void test();
	bool getTestOn();
	void updateIo();
};
*/

clsBattery::clsBattery(){
	state=0;
	testOn=FALSE;
	tick=-1;
	replace=FALSE;
	testByUserRequest=FALSE;
}

void clsBattery::timer(){
	tick++;
	if(tick==0){
		test();
	}
	else if(tick>=60){		//Each minute
		test();
		tick=0;
	}
}

void clsBattery::test(){
	typeAeiTime now;
	static int tickTest;
	switch(state){
	case 0:
		//First time test state machine is called to establish initial conditions
		updateAcPowerStatus();
		updateDcPowerStatus();
		state=1;
		break;
	case 1:
		if(!replace){								//check if it is time to alarma for battery replacement
			typeAeiTime installDate;
			getNowTime(& now);
			installDate.sysTime=(time_t)getBattInstallDate();
			//iprintf("\nFecha de instalación %llu", installDate.sysTime);
			time_t timeElapsed=now.sysTime-installDate.sysTime;
			//iprintf("\nTiempo transcurrido %llu", timeElapsed);
			if(timeElapsed>15552000){					//It has elapsed 6 months, it is time to replace battery
				replace=TRUE;
				if(stMac30.getStatusIdx(idxBattery)!='F')
					stMac30.setStatus(idxBattery, 'W', 72);
				logg.newPrint("\nBattery replacement");
			}
			else{
				if(stMac30.getStatusIdx(idxBattery)!='F')
					stMac30.setStatus(idxBattery, 'N', 71);
			}
		}

		//check if it is time to make battery test
		if(getBattTestOnOff()){								//Test is enabled
			if(getBattTestHour()==getHourOfDay()){			//It is the hour to make test
				typeAeiTime lastDateTest;
				getNowTime(& now);
				time_t period=((time_t)getBattTestPeriod())*86400;
				lastDateTest.sysTime=(time_t)getBattTestLast();
				time_t timeElapsed=now.sysTime-lastDateTest.sysTime;
				if(timeElapsed>period){					//It has elapsed time to make test since last time it was done
					setBattTestLast(now.sysTime);
					logg.newPrint("\nBattery test");
					state=2;
				}
			}
		}

		if(testByUserRequest){
			testByUserRequest=FALSE;
			logg.newPrint("\n Battery test by user request");
			state=2;
		}

		break;
	case 2:										//ON Battery test
		tickTest=0;
		ioDigOutputControl(0, outLatchON, 0);
		testOn=TRUE;
		state=3;
		break;
	case 3:										//Battery running for 30 minutes
		if(++tickTest>30){
			ioDigOutputControl(0, outLatchOFF, 0);
			testOn=FALSE;
			stMac30.setStatus(idxBattery, 'N', 71);
			state=0;
		}
		else{
			int batteryLow;
			batteryLow=digDB.getDigState(13);
			if(batteryLow==1){
				ioDigOutputControl(0, outLatchOFF, 0);
				testOn=FALSE;
				stMac30.setStatus(idxBattery, 'W', 73);
				state=0;
			}
		}
		break;
	}

}

bool clsBattery::getTestOn(){
	return testOn;
}

void clsBattery::updateIo(){
	int batteryLow;

	batteryLow=digDB.getDigState(13);
	if(batteryLow==1){
		if(!testOn){
			stMac30.setStatus(idxBattery, 'F', 74);
		}
	}
}

/***************************************** END BATTERY TEST PROCESS ****************************************************/


/***************************************** IO UTILITIES PROCESS ****************************************************/


void updateAcPowerStatus(){
	int acPowerSt;

	if(digDB.getDigState(10)==NOT_UPDATED || digDB.getDigState(14)==NOT_UPDATED)
		return;
	if(battery.getTestOn()==TRUE)
		return;

	acPowerSt=digDB.getDigState(10)+(digDB.getDigState(14)<<1);
	switch(acPowerSt){
		case 0: default:
			stMac30.setStatus(idxAcPower, 'F', 54);
			break;
		case 1:
			stMac30.setStatus(idxAcPower, 'W', 53);
			break;
		case 2:
			stMac30.setStatus(idxAcPower, 'W', 52);
			break;
		case 3:
			stMac30.setStatus(idxAcPower, 'N', 51);
			break;
	}

}

void updateDcPowerStatus(){
	int dcPowerSt;

	if(digDB.getDigState(12)==NOT_UPDATED || digDB.getDigState(11)==NOT_UPDATED)
		return;
	if(battery.getTestOn()==TRUE)
		return;

	dcPowerSt=digDB.getDigState(12)+(digDB.getDigState(11)<<1);
	switch(dcPowerSt){
		case 0: default:
			stMac30.setStatus(idxDcPower, 'F', 64);
			break;
		case 1:
			stMac30.setStatus(idxDcPower, 'W', 63);
			break;
		case 2:
			stMac30.setStatus(idxDcPower, 'W', 62);
			break;
		case 3:
			stMac30.setStatus(idxDcPower, 'N', 61);
			break;
	}

}

void updateDoorStatus(){
	int doorState;
	doorState=digDB.getDigState(3);
	if(doorState==1)
		stMac30.setStatus(idxDoorSt, 'N', 41);
	else
		stMac30.setStatus(idxDoorSt, 'W', 42);
}

void attnIoUtilities(digInput * dig){
	switch(dig->bit){
		case 3:	//Door
			updateDoorStatus();
		break;

		case 10:								//AC power supply 1
		case 14:								//AC power supply 2
			void updateAcPowerStatus();
		break;

		case 12:								//DC power supply 1
		case 11:								//DC power supply 2
			void updateDcPowerStatus();
		break;

		case 15:								//Battery power supply 1
			//This input is not used because battery relay was latched mechanically to NO position
		break;

		case 13:								//Battery power supply 2
			battery.updateIo();
		break;

	}
}


void startIoUtilities(){
	setFnsAttnProcess(sgGetDigInputBit(idxDoor), procIoUtilities, attnIoUtilities);			//Door
	setFnsAttnProcess(sgGetDigInputBit(idxAcOk1), procIoUtilities, attnIoUtilities);		//AC power supply 1
	setFnsAttnProcess(sgGetDigInputBit(idxAcOk2), procIoUtilities, attnIoUtilities);		//AC power supply 2
	setFnsAttnProcess(sgGetDigInputBit(idxDcOk1), procIoUtilities, attnIoUtilities);		//DC power supply 1
	setFnsAttnProcess(sgGetDigInputBit(idxDcOk2), procIoUtilities, attnIoUtilities);		//DC power supply 2
	//setFnsAttnProcess(15, procIoUtilities, attnIoUtilities);		//Battery power supply 1
	setFnsAttnProcess(sgGetDigInputBit(idxBat2), procIoUtilities, attnIoUtilities);		//Battery power supply 2
}

/***************************************** EN SPEED PPROCESS ****************************************************/


/***************************************************** Class speed **********************************************

class speed {
public:
	uint32_t value;
	int direction;

	speed();
	uint32_t getValue();
	int getDir();
	void updateSpeed(uint32_t _speed, int _direction);
	void reset();
};
*/

speed::speed(){
	value=0;
	direction=0;
}

uint32_t speed::getValue(){
	return value;
}

int speed::getDir(){
	return direction;
}

void speed::updateSpeed(uint32_t _speed, int _direction){
	value=_speed;
	direction=_direction;
}

void speed::reset(){
	value=0;
	direction=0;
}



/***************************************************** Class status **********************************************


#define MAX_STATUS_PAR		7
enum stIndex {idxMac30, idxTagReader, idxAxleDet, idxLoopDet, idxAcPower, idxBattery, idxDoor};

typedef struct{
	char id[21];
	char status;
	int code;
	typeAeiTime t;
}stData;

class status {
public:
	stData par[MAX_STATUS_PAR];

	status();
	const char * getStatus();
	void setStatus(int index, char _status, int _code);
	char getStatusIdx(int index);
	void updateStatus();
};

*/

clsStatus::clsStatus(){
	strcpy(par[idxMac30].id,"MAC30\0");
	strcpy(par[idxTagReader].id,"TAG READER\0");
	strcpy(par[idxAxleDet].id,"AXLE DETECTOR\0");
	strcpy(par[idxLoopDet].id,"LOOP DETECTOR\0");
	strcpy(par[idxDoorSt].id,"DOOR");
	strcpy(par[idxAcPower].id,"AC POWER\0");
	strcpy(par[idxDcPower].id,"DC POWER\0");
	strcpy(par[idxBattery].id,"BATTERY\0");
	for(int i=0; i<MAX_STATUS_PAR; i++){
		par[i].code=0;
		par[i].status='W';
	}
}

char statusCustom[150];
const char * clsStatus::getStatus(){
	static int dummyCode=100;
	char code[4];

	memset(code, '\0',4);
	memset(statusCustom, '\0', 150);

	updateStatus();

	strcpy(statusCustom, "source:");
	strcat(statusCustom, getAeiID());
	for(int i=0; i<MAX_STATUS_PAR; i++){
		sprintf(code, "%2d", par[i].code);

		strcat(statusCustom, "|");
		strcat(statusCustom, par[i].id);
		strcat(statusCustom, ":");
		strncat(statusCustom, &par[i].status, 1);
		strcat(statusCustom, ":");
		strcat(statusCustom, code);
	}
	sprintf(code, "%d", dummyCode);
	strcat(statusCustom, "|DUMMY:N:");
	strcat(statusCustom, code);
	strcat(statusCustom, "|");

	if(++dummyCode>199)
		dummyCode=100;
	return statusCustom;
}

void clsStatus::setStatus(int _index, char _status, int _code){
	par[_index].status=_status;
	par[_index].code=_code;
	getNowTime(& par[_index].t);
}

char clsStatus::getStatusIdx(int _index){
	return par[_index].code;
}

void clsStatus::updateStatus(){
	updateDoorStatus();
	updateAcPowerStatus();
	updateDcPowerStatus();
	battery.updateIo();
}
