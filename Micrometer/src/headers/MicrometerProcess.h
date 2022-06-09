/*
 * MicrometerProcess.h
 *
 *  Created on: 22 sep 2021
 *      Author: Integra Fredy
 */

#ifndef HEADERS_MICROMETERPROCESS_H_
#define HEADERS_MICROMETERPROCESS_H_
#include <basictypes.h>
#include "headers/IO_Interface.h"
#include "fifo/Fifo.h"

struct InputsInfo{
	volatile uint8_t inputNumber;
	volatile IOTime time;
	volatile uint8_t state;
	volatile char * alias;
};

struct InputsInfoMod{
	uint8_t inputNumber;
	IOTime time;
	uint8_t state;
	char * alias;
};

enum ProcessState { closeProcess, openProcess, noProcess, startedTest, runningTest, manualTest, stopManual, modeChanged, badMode};
const char ProcessInfo[9][20] = {"Cerrado","Abierto", "No conocido", "Espera", "Prueba", "Prueba manual", "Prueba detenida", "Cambio de modo", "Sin cambio"};

enum TestStatus { endTest,noCorrectFasesStatus, noFasesError, fasesError, noDataTest};
const char TestInfo[5][60] = {"Finalizada", "Prueba no iniciada, fases en estado diferente al inicial", "Sin error de fases", "Error de fases", "No hay datos de fases ni comandos"};

enum HourMeterMode { hourmeterMode, externMode};
const char ModeInfo[2][12] = {"Horometro","Externo"};

typedef struct{
	char fases[50];
	IOTime diference;
	ProcessState state;
	int cycleNumber;
	char alert[10];
}PhasesDiferences;

class MicroProcess {
public:
	MicroProcess();
	void waitUpdate();
	void update(DigInput * dig);
	TestStatus startTest();
	TestStatus startMicroProcess(ProcessState state, bool saveData);
	TestStatus startManualTest();
	TestStatus abortManualTest();
	TestStatus getProcess(ProcessState state, int cycleNumber);
	bool makeCSVHeader(bool startSDandFile, char * testType);
	void printInformation();
	void saveDataInto(char * Data, bool intoWeb, bool intoSd);
	void setDataToSD(char * Data);
	ProcessState getProcessState();
	void getWebResults(Fifo *webData);

};

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
	void getMicrometerFileData(char *fileName, char *fileExt);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* HEADERS_MICROMETERPROCESS_H_ */
