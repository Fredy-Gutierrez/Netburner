/*
 * IO_Interface.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */



#ifndef LIBS_IO_INTERFACE_H_
#define LIBS_IO_INTERFACE_H_

#include <basictypes.h>
#include "headers/OSTime.h"
#include "headers/Default_Values.h"


struct Digitals{
	uint8_t port;
	uint8_t pin;
	uint8_t report;
	char alias[ALIASDIGITALSIZE];
};

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
	void initIO();
	void AeiIoScan();
	void ioStart();
	void ioStop();
	void setIOPulse(int ioDig, int ms);
	void getDigitalsInfo(Digitals * digitals);
	void printDigitals();
#ifdef __cplusplus
}
#endif /* __cplusplus */


#define MAX_DIG_INPUTS 			5
#define MAX_DIG_OUTPUTS 		2
#define NOT_UPDATED				0x55
#define FILTER_TICKS			5
#define ALL_DIGS				0xFF
#define OUTPUTS_PIN_ENABLE		8

enum typeProcess {procLoopDet, procAxeDet, procSpeed, procIoUtilities};
enum typeOutFunction {outNoFunction, outLatchON, outLatchOFF, outPulseON, outPulseOFF};
enum typeStateOutput {outStOFF, outStON, outStPulseON, outStPulseOFF};

/***************************************************** Class digInput ***********************************************/
class DigInput {
public:
	volatile uint32_t stateField;
	volatile uint32_t stateLast;
	volatile uint32_t state;
	volatile uint32_t filterTicks;
	volatile uint32_t port;
	volatile uint32_t pin;
	volatile uint32_t bit;
	volatile bool repChg;
	char alias[ALIASDIGITALSIZE]={'N','U','L','L',0};
	volatile IOTime chgTime;
	void (* userChgReport)(DigInput * dig);
	DigInput();
	void construct(uint32_t _bit, uint32_t _port, uint32_t _pin, char* alias);
	void update();
};

/***************************************************** Class digOutput ***********************************************/
class DigOutput {
public:
	volatile typeStateOutput state;
	volatile uint32_t valueField;
	volatile uint32_t function;
	volatile uint32_t ticks;
	volatile uint32_t ticksCount;
	volatile uint32_t port;
	volatile uint32_t pin;
	volatile uint32_t bit;
	volatile bool repChg;
	char alias[ALIASDIGITALSIZE]={'N','U','L','L',0};
	volatile IOTime chgTime{ 0, 0, 0 };
	void (* userChgReport)(DigOutput * dig);

	DigOutput();
	void construct(uint32_t _bit, uint32_t _port, uint32_t _pin, char* aliasDig);
	void control(typeOutFunction fun, uint32_t time);
	void update();
};

/***************************************************** Class digDataBase ***********************************************/
class DigDataBase {
public:
	DigInput digInputs[MAX_DIG_INPUTS];
	DigOutput digOutputs[MAX_DIG_OUTPUTS];
	DigDataBase();
	void progPins();
	void updatePrnChange(uint32_t bit, bool prnChange);
	void susRepChg(uint32_t dig, void (* fnChgReport)(DigInput * dig));
	int getDigState(uint32_t bit);
	void setOnPulse(int outPut, int ms);
};
extern DigDataBase digDB;

#include "headers/MicrometerProcess.h"
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
	struct InputsInfo *getDigitals();
	void setFnsAttnProcess(uint32_t digNumber, void (* fn)(DigInput * dig));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBS_IO_INTERFACE_H_ */



