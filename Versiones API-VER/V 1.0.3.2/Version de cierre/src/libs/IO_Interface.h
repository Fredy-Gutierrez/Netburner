/*
 * IO_Interface.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#ifndef LIBS_IO_INTERFACE_H_
#define LIBS_IO_INTERFACE_H_

#include <basictypes.h>
#include "libs/Rtc_Controller.h"

#define MAX_DIG_INPUTS 			18
#define MAX_DIG_OUTPUTS 		2
#define NOT_UPDATED				0x55
#define FILTER_TICKS			5
#define ALL_DIGS				0xFF
#define OUTPUTS_PIN_ENABLE		8
#define INVALID_ALIAS			MAX_DIG_INPUTS

enum typeProcess {procLoopDet, procAxleDet, procIoUtilities};
enum typeOutFunction {outNoFunction, outLatchON, outLatchOFF, outPulseON, outPulseOFF};
enum typeStateOutput {outStOFF, outStON, outStPulseON, outStPulseOFF};

enum idxIoSignals {idxSys1, idxSys2, idxSysP1, idxSysP2, idxSysN1, idxSysN2, idxSysPN1, idxSysPN2,
					idxLoopC_I, idxLoopO, idxDcOk1, idxDcOk2, idxAcOk1, idxAcOk2, idxBat1, idxBat2,
					idxDoor, idxFree};
#define	idxSys1_S	idxSysN1
#define	idxSysP1_S	idxSysPN1

typedef struct {
	const char * id;
	int refDigInput;
	uint32_t bit;
}tyIoSignal;

#define MAX_IO_SIGNALS		idxFree + 1

/***************************************************** Class digInput **********************************************
*/
class digInput {
public:
	volatile uint32_t stateField;
	volatile uint32_t stateLast;
	volatile uint32_t state;
	volatile uint32_t filterTicks;
	volatile uint32_t port;
	volatile uint32_t pin;
	volatile uint32_t bit;
	volatile bool repChg;
	volatile typeAeiTime chgTime;
	void (* userChgReport)(digInput * dig);

	digInput();
	void construct(uint32_t _bit, uint32_t _port, uint32_t _pin);
	void update();
};

/***************************************************** Class digOutput **********************************************
*/
class digOutput {
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
	volatile typeAeiTime chgTime;
	void (* userChgReport)(digOutput * dig);

	digOutput();
	void construct(uint32_t _bit, uint32_t _port, uint32_t _pin);
	void control(typeOutFunction fun, uint32_t time);
	void update();
};

/***************************************************** Class digDataBase **********************************************
*/
class digDataBase {
public:
	digInput digInputs[MAX_DIG_INPUTS];
	digOutput digOutputs[MAX_DIG_OUTPUTS];

	digDataBase();
	void progPins();
	void updatePrnChange(uint32_t bit, bool prnChange);
	void susRepChg(uint32_t dig, void (* fnChgReport)(digInput * dig));
	int getDigState(uint32_t bit);

};

extern digDataBase digDB;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
	void initIO();
	void AeiIoScan();
	void SaveTicks(unsigned long ticks);
	void ioStart();
	void ioStop();
	void ioDigOutputControl(uint32_t bit, typeOutFunction function, uint32_t time);
	void ioPrnChgDisable(uint32_t bit);
	void ioPrnChgEnable(uint32_t bit);
	void printTicks(void);
	void printDigitals(void);
	const char *ioGetDigInputId(uint32_t _bit);
	uint32_t sgGetDigInputBit(int idx);

	void setDigInputsBit(tyIoSignal newInput, idxIoSignals index);
	void setFnsAttnProcess(uint32_t digNumber, typeProcess process, void (* fn)(digInput * dig));

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */

extern digDataBase digDB;

#endif /* LIBS_IO_INTERFACE_H_ */


