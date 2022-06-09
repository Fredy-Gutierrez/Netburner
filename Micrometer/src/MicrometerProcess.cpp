/*
 * MicrometerProcess.cpp
 *
 *  Created on: 22 sep 2021
 *      Author: Integra Fredy
 */
#include "headers/MicrometerProcess.h"
#include "FatLib/FileSystemUtils.h"
#include "headers/Configurations.h"
#include "headers/Utilities.h"
#include "headers/Rtc_Controller.h"
#include <stdio.h>
#include <nbrtos.h>
#include <HiResDelay.h>
#include <stdlib.h>
#include <string.h>

Fifo fifoProc;
Fifo fifoDiferences;
Fifo webResult;
char* pfileExt = "csv";
char lastFileName[MAX_FILE_NAME];

FN_FILE * filePointer = NULL;
ProcessState generalState = noProcess;

volatile unsigned long long timeWait = 0;
bool pulseDetected = false;
bool manualTestFlag = false;

MicroProcess::MicroProcess(){

}

volatile bool timeStarted = false;

void MicroProcess::waitUpdate(){
	if(timeStarted){
		if(timeWait > 0){
			timeWait--;
		}
	}
}

void MicroProcess::update(DigInput * dig){
	if( !(dig->bit < (MAX_DIG_INPUTS - CMDDIGITALSIN)) && !timeStarted ){
		startGettingTime();
		timeStarted = true;
		pulseDetected = true;
	}
	struct InputsInfo *newInfo = (struct InputsInfo *)malloc (sizeof (struct InputsInfo));
	newInfo->inputNumber = dig->bit;
	newInfo->state = dig->state;
	newInfo->time.Secs = dig->chgTime.Secs;
	newInfo->time.msSec = dig->chgTime.msSec;
	newInfo->time.uSec = dig->chgTime.uSec;
	newInfo->alias = dig->alias;
	fifoProc.Add(newInfo);
}

TestStatus MicroProcess::startMicroProcess(ProcessState state, bool saveData){
	generalState = runningTest;
	webResult.Initiate();
	char Data[MAXBUFFERSIZE];
	if(saveData){
		sniprintf(Data, MAXBUFFERSIZE, "%s %s","Prueba de",ProcessInfo[state]);
		makeCSVHeader(true, Data);
	}
	int digNum = 0;
	switch(state){
		case closeProcess:
			digNum = CLOSEDIGITALOUT;
			break;
		case openProcess:
			digNum = OPENDIGITALOUT;
			break;
		default:
			break;
	}
	fifoDiferences.Initiate();//clean fifo with test information
	DelayObject delay;
	fifoProc.Initiate();
	pulseDetected = false;

	if(state == closeProcess){
		setIOPulse(digNum, getTClosePulseTime());
	}else{
		setIOPulse(digNum, getTOpenPulseTime());
	}

	unsigned long long maxWaitAfterPulse = (getTMaxTimeAfterPulse() * 1000)/50;
	timeWait = (getTPhasesWaitTime() * 1000)/200;
	while(timeWait > 0 && maxWaitAfterPulse > 0){
		delay.DelayUsec( 50 );
		if(!pulseDetected){
			maxWaitAfterPulse--;
		}
	}
	stopGettingTime();
	timeStarted = false;
	if(saveData){
		TestStatus processResult = getProcess(state, 1);
		if(processResult != noFasesError){
			if(processResult == noDataTest){
				sniprintf(Data, MAXBUFFERSIZE, "%s",TestInfo[noDataTest]);
				saveDataInto(Data, true, true);
			}
			printInformation();
			closeFile(filePointer);
			generalState = noProcess;
			return fasesError;
		}
		printInformation();
		closeFile(filePointer);
		filePointer = NULL;
	}
	generalState = noProcess;
	return endTest;
}

TestStatus MicroProcess::startTest(){
	generalState = runningTest;
	webResult.Initiate();
	char Data[MAXBUFFERSIZE];
	int cycleNumber = getTCycleNumber();
	ProcessState state = getProcessState();//get the actual phases state to know which state will be execute
	ProcessState endCycleState = state;
	if(state != getTInitialState()){
		sniprintf(Data, MAXBUFFERSIZE, "%s",TestInfo[noCorrectFasesStatus]);
		saveDataInto(Data, true, false);
		generalState = noProcess;
		return noCorrectFasesStatus;
	}

	sniprintf(Data, MAXBUFFERSIZE, "%s %d ciclos","Prueba", cycleNumber);
	makeCSVHeader(true, Data);

	fifoDiferences.Initiate();//clean fifo with test information
	int actualCycle = 1;
	while(cycleNumber > 0){
		if(state == closeProcess){
			state = openProcess;
		}else{
			state = closeProcess;
		}
		int digNum = 0;
		switch(state){
			case closeProcess:
				digNum = CLOSEDIGITALOUT;
				break;
			case openProcess:
				digNum = OPENDIGITALOUT;
				break;
			default:
				break;
		}
		/***********************TASKSTART************************/
		DelayObject delay;
		fifoProc.Initiate();
		pulseDetected = false;

		if(state == closeProcess){
			setIOPulse(digNum, getTClosePulseTime());
		}else{
			setIOPulse(digNum, getTOpenPulseTime());
		}
		//setIOPulse(digNum, getTClosePulseTime());

		unsigned long long maxWaitAfterPulse = (getTMaxTimeAfterPulse() * 1000)/50;
		timeWait = (getTPhasesWaitTime() * 1000)/200;
		while(timeWait > 0 && maxWaitAfterPulse > 0){
			delay.DelayUsec( 50 );
			if(!pulseDetected){
				maxWaitAfterPulse--;
			}
		}

		stopGettingTime();
		timeStarted = false;
		TestStatus processResult = getProcess(state, actualCycle);
		if(processResult != noFasesError){
			if(processResult == noDataTest){
				sniprintf(Data, MAXBUFFERSIZE, "%s",TestInfo[noDataTest]);
				saveDataInto(Data, true, true);
			}
			printInformation();
			closeFile(filePointer);
			//closeSDCard();
			generalState = noProcess;
			return fasesError;
		}
		if(state == endCycleState){
			actualCycle++;
			cycleNumber--;
			if(cycleNumber > 0){
				OSTimeDly(TICKS_PER_SECOND*getTCycleWaitTime());
			}
		}else{
			OSTimeDly(TICKS_PER_SECOND*getTCmdWaitTime());
		}
	}
	printInformation();
	closeFile(filePointer);
	//closeSDCard();
	generalState = noProcess;
	return endTest;
}

TestStatus MicroProcess::startManualTest(){
	generalState = runningTest;
	webResult.Initiate();
	char Data[MAXBUFFERSIZE];

	sniprintf(Data, MAXBUFFERSIZE, "%s","Prueba manual");
	makeCSVHeader(true, Data);

	fifoDiferences.Initiate();//clean fifo with test information
	int actualCycle = 1;
	manualTestFlag = true;
	while(manualTestFlag){
		/***********************TASKSTART************************/
		DelayObject delay;
		fifoProc.Initiate();
		pulseDetected = false;
		timeWait = (getTPhasesWaitTime() * 1000)/200;
		int abortTicks = 4;
		while(timeWait > 0){
			delay.DelayUsec( 50 );
			if(!manualTestFlag){
				abortTicks--;
				if(!(abortTicks > 0)){
					timeWait = 0;
				}
			}
		}
		stopGettingTime();
		timeStarted = false;
		ProcessState state = getProcessState();
		TestStatus processResult = getProcess(state, actualCycle);
		if(processResult != noFasesError){
			if(manualTestFlag){
				if(processResult == noDataTest){
					sniprintf(Data, MAXBUFFERSIZE, "%s",TestInfo[noDataTest]);
					saveDataInto(Data, true, true);
				}
				printInformation();
				closeFile(filePointer);
				//closeSDCard();
				generalState = noProcess;
				manualTestFlag = false;
				return fasesError;
			}
		}
	}

	printInformation();
	closeFile(filePointer);
	generalState = noProcess;
	return endTest;
}

TestStatus MicroProcess::abortManualTest(){
	if(manualTestFlag){
		manualTestFlag = false;
	}
	return endTest;
}

TestStatus MicroProcess::getProcess(ProcessState state, int cycleNumber){
	int size = fifoProc.FifoSize();
	if(size > 0){
		int arrayInfoCount = 0;
		InputsInfoMod arrayInfo[size];
		void * point = fifoProc.Get();
		while(point){
			struct InputsInfo *newInfo = (struct InputsInfo *)point;
			arrayInfo[arrayInfoCount].alias = (char*)newInfo->alias;
			arrayInfo[arrayInfoCount].inputNumber = (uint8_t)newInfo->inputNumber;
			arrayInfo[arrayInfoCount].state = (uint8_t)newInfo->state;
			arrayInfo[arrayInfoCount].time.Secs = (unsigned long long)newInfo->time.Secs;
			arrayInfo[arrayInfoCount].time.msSec = (uint32_t)newInfo->time.msSec;
			arrayInfo[arrayInfoCount].time.uSec = (uint32_t)newInfo->time.uSec;
			arrayInfoCount++;
			fifoProc.MoveReader(true);
			point = fifoProc.Get();
		}
		/**********************************PROCESS TO GET DIFERENCES***********************************/
		if(arrayInfoCount > 0){
			Digitals digitals[MAX_DIG_INPUTS];
			getDigitalsInfo(digitals);
			orderByAsc(arrayInfo, size);
			for (int x = 0; x < arrayInfoCount; ++x){
				if(arrayInfo[x].inputNumber < (MAX_DIG_INPUTS - CMDDIGITALSIN)){
					PhasesDiferences *newDif = (PhasesDiferences *)malloc (sizeof (PhasesDiferences));
					strcpy(newDif->fases, arrayInfo[x].alias);
					newDif->diference.Secs = arrayInfo[x].time.Secs;
					newDif->diference.msSec = arrayInfo[x].time.msSec;
					newDif->diference.uSec = arrayInfo[x].time.uSec;
					newDif->cycleNumber = cycleNumber;
					newDif->state = state;
					strcpy(newDif->alert, "-");
					fifoDiferences.Add(newDif);
				}
				for(int y = 0; y < MAX_DIG_INPUTS;y++){
					if(arrayInfo[x].inputNumber == digitals[y].pin){
						digitals[y].report = 1;
					}
				}
			}


			/*************************************************************DIFERENCES**********************************************************/
			for (int x = 0; x < arrayInfoCount; ++x) {
				if(arrayInfo[x].inputNumber < (MAX_DIG_INPUTS - CMDDIGITALSIN)){
					if( (x+1) < arrayInfoCount && arrayInfo[x+1].inputNumber < (MAX_DIG_INPUTS - CMDDIGITALSIN)){
						PhasesDiferences *newDif = (PhasesDiferences *)malloc (sizeof (PhasesDiferences));
						strcpy(newDif->fases, "DIFERENCIA ");
						strcat(newDif->fases, arrayInfo[x].alias);
						strcat(newDif->fases, "-");
						strcat(newDif->fases, arrayInfo[x+1].alias);
						newDif->cycleNumber = cycleNumber;
						newDif->state = state;
						IOTime timeIni;
						timeIni.Secs = arrayInfo[x].time.Secs;
						timeIni.msSec = arrayInfo[x].time.msSec;
						timeIni.uSec = arrayInfo[x].time.uSec;
						IOTime timeEnd;
						timeEnd.Secs = arrayInfo[x+1].time.Secs;
						timeEnd.msSec = arrayInfo[x+1].time.msSec;
						timeEnd.uSec = arrayInfo[x+1].time.uSec;
						getIOTimeDiference(&timeIni, &timeEnd, &newDif->diference );
						if(state == closeProcess){
							if(newDif->diference.msSec >= getTMaxDiffClose() || (newDif->diference.Secs * 1000)>=getTMaxDiffClose()){
								strcpy(newDif->alert, "Alerta");
							}else{
								strcpy(newDif->alert, "-");
							}
						}else if(state == openProcess){
							if(newDif->diference.msSec >= getTMaxDiffOpen() || (newDif->diference.Secs * 1000) >= getTMaxDiffOpen() ){
								strcpy(newDif->alert, "Alerta");
							}else{
								strcpy(newDif->alert, "-");
							}
						}
						fifoDiferences.Add(newDif);
					}
				}
				if(arrayInfo[x].inputNumber < (MAX_DIG_INPUTS - CMDDIGITALSIN) && (x+2) < arrayInfoCount && arrayInfo[x+2].inputNumber < (MAX_DIG_INPUTS - CMDDIGITALSIN)){
					PhasesDiferences *newDif = (PhasesDiferences *)malloc (sizeof (PhasesDiferences));
					strcpy(newDif->fases, "DIFERENCIA ");
					strcat(newDif->fases, arrayInfo[x].alias);
					strcat(newDif->fases, "-");
					strcat(newDif->fases, arrayInfo[x+2].alias);
					newDif->cycleNumber = cycleNumber;
					newDif->state = state;
					IOTime timeEnd;
					timeEnd.Secs = arrayInfo[x+2].time.Secs;
					timeEnd.msSec = arrayInfo[x+2].time.msSec;
					timeEnd.uSec = arrayInfo[x+2].time.uSec;
					IOTime timeIni;
					timeIni.Secs = arrayInfo[x].time.Secs;
					timeIni.msSec = arrayInfo[x].time.msSec;
					timeIni.uSec = arrayInfo[x].time.uSec;
					getIOTimeDiference(&timeIni, &timeEnd, &newDif->diference );
					if(state == closeProcess){
						if(newDif->diference.msSec >= getTMaxDiffClose() || (newDif->diference.Secs * 1000)>=getTMaxDiffClose()){
							strcpy(newDif->alert, "Alerta");
						}else{
							strcpy(newDif->alert, "-");
						}
					}else if(state == openProcess){
						if(newDif->diference.msSec >= getTMaxDiffOpen() || (newDif->diference.Secs * 1000) >= getTMaxDiffOpen() ){
							strcpy(newDif->alert, "Alerta");
						}else{
							strcpy(newDif->alert, "-");
						}
					}
					fifoDiferences.Add(newDif);
				}
			}
			/*************************************************Check the inputs recived*********************************/
			char Data[MAXBUFFERSIZE];
			bool cmdDet = false;
			bool badPhase = false;
			for(int y = 0; y < MAX_DIG_INPUTS;y++){
				if(digitals[y].pin < (MAX_DIG_INPUTS - CMDDIGITALSIN)){
					if(digitals[y].report != 1){
						sniprintf(Data, MAXBUFFERSIZE, "Entrada %s error",digitals[y].alias);
						saveDataInto(Data, true, true);
						badPhase = true;
					}
				}else{
					if(digitals[y].report == 1){
						cmdDet = true;
					}
				}
			}
			if(!cmdDet){
				sniprintf(Data, MAXBUFFERSIZE, "Error en la entrada del comando");
				saveDataInto(Data, true, true);
				return fasesError;
			}
			if(badPhase){
				return fasesError;
			}
			return noFasesError;
		}else{
			return noDataTest;
		}
	}
	return noDataTest;
}

bool MicroProcess::makeCSVHeader(bool startSDandFile, char * testType){
	char Data[MAXBUFFERSIZE];
	char time[15];
	char date[15];
	if(startSDandFile){
	#if (defined(USE_MMC))
		f_chdrive(MMC_DRV_NUM);
	#endif
		getDateIntoBuffer(date, '-');
		getTimeIntoBuffer(time,'_');
		sniprintf(Data, MAXBUFFERSIZE, "%s %s %s %s", testType, date, time, getDSerialNum());
		filePointer = createAndOpenNewFile(Data, pfileExt, "w+", lastFileName);
	}
	sniprintf(Data, MAXBUFFERSIZE, "TIPO DE PRUEBA: %s\n",testType);
	setDataToSD(Data);
	sniprintf(Data, MAXBUFFERSIZE, "MARCA: %s,TIPO: %s,NO.SERIE: %s,,OPERADOR: %s,SUBESTACION: %s,EQUIPO: %s,No. SERIE %s,MARCA: %s,CAPACIDAD: %s,TIPO: %s,TENSION: %s,No. REPORTE: %d,DIVISION: %s,ZONA: %s\n"
			,getHMaker(),getHType(),getHSerialNum(), getDOperator(),getDSubStation(), getDDevice(),getDSerialNum(),getDMaker(),getDCapacity(),getDType(), getDTension(), (int)getDReportNum(),getDDivision(),getDZone());
	setDataToSD(Data);
	return true;
}

void MicroProcess::printInformation(){
	if(fifoDiferences.FifoSize() > 0){
		char Data[MAXBUFFERSIZE];
		/*Header*/
		sniprintf(Data, MAXBUFFERSIZE, "%s","FASE,SEGUNDOS,MILISEGUNDOS,MICROSEGUNDOS,APERTURA,CIERRE,CICLO,ALERTA\n");
		setDataToSD(Data);
		iprintf("FASE,SEGUNDOS,MILISEGUNDOS,MICROSEGUNDOS\r\n");

		PhasesDiferences *difGeted = (PhasesDiferences *)fifoDiferences.Get();
		while(difGeted){
			bool open = false, close = false;
			if(difGeted->state == closeProcess){ close = true; }else { open = true; }
			sniprintf(Data, MAXBUFFERSIZE, "%s,%llu,%lu,%03lu,%d,%d,%d,%s\n",difGeted->fases,difGeted->diference.Secs,difGeted->diference.msSec,difGeted->diference.uSec,open,close,difGeted->cycleNumber,difGeted->alert);
			setDataToSD(Data);

			char * webData = (char *)malloc(sizeof(char) * MAXBUFFERSIZE);
			sniprintf(webData, MAXBUFFERSIZE, "<tr> <td>%s</td> <td>%llu</td> <td>%lu</td> <td>%03lu</td> <td>%d</td> <td>%d</td> <td>%d</td> <td>%s</td> </tr>",difGeted->fases,difGeted->diference.Secs,difGeted->diference.msSec,difGeted->diference.uSec,open,close,difGeted->cycleNumber,difGeted->alert);
			webResult.Add(webData);

			iprintf("%s %llu:%lu:%03lu\n",difGeted->fases,difGeted->diference.Secs,difGeted->diference.msSec,difGeted->diference.uSec);

			fifoDiferences.MoveReader(true);
			difGeted = (PhasesDiferences *)fifoDiferences.Get();
		}
	}
}

void MicroProcess::saveDataInto(char * Data, bool intoWeb, bool intoSd){
	char sdData[MAXBUFFERSIZE];
	if(intoSd){
		sniprintf(sdData, MAXBUFFERSIZE, "%s\n",Data);
		setDataToSD(sdData);
	}
	if(intoWeb){
		char * webData = (char *)malloc(sizeof(char) * MAXBUFFERSIZE);
		sniprintf(webData, MAXBUFFERSIZE, "<tr>%s</tr>",Data);
		webResult.Add(webData);
	}
}

void MicroProcess::setDataToSD(char * Data){
	if(filePointer != NULL){
		writeText((uint8_t *)Data, strlen(Data), filePointer);
	}
}

ProcessState MicroProcess::getProcessState(){
	ProcessState state = openProcess;
	struct InputsInfo *newInfo = getDigitals();
	for (int x = 0; x < MAX_DIG_INPUTS - CMDDIGITALSIN; ++x) {
		if(newInfo[x].state == 1){
			state = closeProcess;
		}
	}
	free(newInfo);
	newInfo = NULL;
	return state;
}


void MicroProcess::getWebResults(Fifo *webData){
	if(webResult.FifoSize() > 0){
		void * pd = webResult.Get();
		while(pd){
			webData->Add(pd);
			webResult.MoveReader(false);
			pd = webResult.Get();
		}
	}
}

void getMicrometerFileData(char *fileName, char *fileExt){
	strcpy(fileName, lastFileName);
	strcpy(fileExt, pfileExt);
}
