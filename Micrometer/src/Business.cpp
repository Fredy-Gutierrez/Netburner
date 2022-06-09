/*
 * Business.cpp
 *
 *  Created on: 22 sep 2021
 *      Author: Integra Fredy
 */
#include "headers/Business.h"
#include <stdio.h>
#include <nbrtos.h>
#include <HiResDelay.h>
#include <ftpd.h>


MicroProcess microProcess;
ProcessState microProcessState = noProcess;
ProcessState state = noProcess;
HourMeterMode meterMode = hourmeterMode;

void attnInputs(DigInput * dig);
void setIOFunct();

void displayMenu(){
	iprintf("\r\n******************Menu******************\r\n");
    iprintf("[O] ABRIR\n");
    iprintf("[C] CERRAR\n");
    iprintf("[S] ESTADO DE FASES\n");
    iprintf("[D] IMPRIMIR DIGITALES\n");
    iprintf("[T] INICIAR PRUEBA\n");
    iprintf("[M] INICIAR PRUEBA MANUAL\n");
    iprintf("[R] REINICIAR\n");
    iprintf("\r\n");
}
void printPhasesState(ProcessState state){
	iprintf("Fases en Estado de %s\r\n", ProcessInfo[state]);
}

void micrometerService(void *pd){

	addFSTask(0, 0);//create fs task for make sd changes using the micrometer

	ProcessState lastState = microProcess.getProcessState();
	while(true){
		switch(microProcessState){
			case noProcess:
				lastState = microProcess.getProcessState();
				break;
			case openProcess:
				microProcessState = runningTest;
				if(lastState != openProcess){
					TestStatus tstate = microProcess.startMicroProcess(openProcess, true);
					iprintf("Estado de prueba: %s\r\n", TestInfo[tstate]);
					state = microProcess.getProcessState();
					lastState = state;
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[lastState]);
				}
				microProcessState = noProcess;
				break;
			case closeProcess:
				microProcessState = runningTest;
				if(lastState != closeProcess){
					TestStatus tstate = microProcess.startMicroProcess(closeProcess, true);
					iprintf("Estado de prueba: %s\r\n", TestInfo[tstate]);
					state = microProcess.getProcessState();
					lastState = state;
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[lastState]);
				}
				microProcessState = noProcess;
				break;
			case runningTest:{
					state = runningTest;
					TestStatus tstate = microProcess.startTest();
					iprintf("Estado de prueba: %s\r\n", TestInfo[tstate]);
					state = microProcess.getProcessState();
					lastState = state;
					microProcessState = noProcess;
					break;
				}
			case manualTest:{
					microProcess.startManualTest();
					microProcessState = noProcess;
					break;
				}
			default:
				break;
		}
		OSTimeDly(TICKS_PER_SECOND);
	}
}

void terminal(void *pd){

	addFSTask(0, 0);//create fs task for make sd changes using the terminal

	state = microProcess.getProcessState();
	displayMenu();
	while(true){
		char c = getchar();
		iprintf("\n\n");
		switch (c){
			case 'O': case'o':
				if(state != openProcess && microProcessState != runningTest && microProcessState != manualTest){
					if(meterMode == hourmeterMode){
						microProcessState = openProcess;
						state = openProcess;
					}else{
						iprintf("Equipo configurado en Modo: %s\r\n", ModeInfo[meterMode]);
					}
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[state]);
				}
				break;
			case 'C': case 'c':
				if(state != closeProcess && microProcessState != runningTest && microProcessState != manualTest){
					if(meterMode == hourmeterMode){
						microProcessState = closeProcess;
						state = closeProcess;
					}else{
						iprintf("Equipo configurado en Modo: %s\r\n", ModeInfo[meterMode]);
					}
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[state]);
				}
				break;
			case 'T': case 't':
				if(microProcessState != runningTest && microProcessState != manualTest){
					if(meterMode == hourmeterMode){
						microProcessState = runningTest;
					}else{
						iprintf("Equipo configurado en Modo: %s\r\n", ModeInfo[meterMode]);
					}
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[microProcessState]);
				}
				break;
			case 'M': case 'm':{
				if(microProcessState != runningTest && microProcessState != manualTest){
					if(meterMode == externMode){
						microProcessState = manualTest;
						bool makingTest = true;
						while(makingTest){
							iprintf("Detener prueba Y/N?\n\n");
							char abort = getchar();
							iprintf("\n\n");
							switch (abort){
								case 'Y': case'y':
									microProcess.abortManualTest();
									makingTest= false;
								break;
							}
						}
					}else{
						iprintf("Equipo configurado en Modo: %s\r\n", ModeInfo[meterMode]);
					}
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[microProcessState]);
				}
				break;
			}
			case 'D': case 'd':
				state = microProcess.getProcessState();
				printPhasesState(state);
				printDigitals();
				break;
			case 'S': case 's':
				state = microProcess.getProcessState();
				printPhasesState(state);
				break;
			case 'R': case 'r':
				reset();
				break;
			case 'F': case 'f':
				FormatExtFlash();
				break;
			default :
				displayMenu();
				break;
		}
	}
}

ProcessState webCommands(ProcessState webState){
	switch(webState){
		case closeProcess:
			if(state != closeProcess && microProcessState != runningTest){
				if(meterMode == hourmeterMode){
					microProcessState = closeProcess;
					state = closeProcess;
					return startedTest;
				}else{
					return badMode;
				}
			}else{
				iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[state]);
				return state;
			}
		case openProcess:
			if(state != openProcess && microProcessState != runningTest){
				if(meterMode == hourmeterMode){
					microProcessState = openProcess;
					state = openProcess;
					return startedTest;
				}else{
					return badMode;
				}
			}else{
				iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[state]);
				return state;
			}
		case startedTest:
			if(microProcessState != runningTest){
				if(meterMode == hourmeterMode){
					microProcessState = runningTest;
					return startedTest;
				}else{
					return badMode;
				}
			}else{
				iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[microProcessState]);
				return microProcessState;
			}
		case manualTest:{
				if(microProcessState != runningTest){
					if(meterMode == externMode){
						microProcessState = manualTest;//microProcess.abortManualTest();
						return startedTest;
					}else{
						return badMode;
					}
				}else{
					iprintf("Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[microProcessState]);
					return microProcessState;
				}
			}
		case stopManual:
			microProcess.abortManualTest();
			return startedTest;
		case modeChanged:
			if(microProcessState != runningTest && microProcessState != manualTest){
				if(meterMode == hourmeterMode){
					meterMode = externMode;
				}else{
					meterMode = hourmeterMode;
				}
				return modeChanged;
			}else{
				return badMode;
			}
			break;
		default:
			return noProcess;
			break;
	}
}

ProcessState getProcessState(bool update){
	if(update){
		state = microProcess.getProcessState();
	}
	return state;
}

ProcessState getMicrometerState(){
	return microProcessState;
}

HourMeterMode getMicrometerMode(){
	return meterMode;
}

void startFtpServer(){
	int status = FTPDStart(21, FTP_PRIO);
	if (status == FTPD_OK)
	{
		iprintf("Started FTP Server\r\n");
	}
	else
	{
		iprintf("** Error: %d. Could not start FTP Server\r\n", status);
	}
}

void startTasks(){
	OSSimpleTaskCreatewName(terminal,TERMINAL_PRIO,"Terminal");
	OSSimpleTaskCreatewName(micrometerService,MICROMETER_PRIO,"Micrometer_Process");
}

void setIOFunct(){
	setFnsAttnProcess(0, attnInputs);
	setFnsAttnProcess(1, attnInputs);
	setFnsAttnProcess(2, attnInputs);
	setFnsAttnProcess(3, attnInputs);
	setFnsAttnProcess(4, attnInputs);
}

void attnWaitTime(){
	microProcess.waitUpdate();
}

void attnInputs(DigInput * dig){
	microProcess.update(dig);
}

void getWebResults(Fifo * webData){
	microProcess.getWebResults(webData);
}

void SDStart(){
	addFSTask(0, 0);
	initSDCard();   // Initialize the CFC or SD/MMC external flash drive
}

void initBusiness(){
	initTimer();
	initIO();
	initI2C_Controller();
	initRtc();
	setIOFunct();
	ioStart();
	startFtpServer();
	SDStart();
	startTasks();
}
