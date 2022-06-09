/*
 * Configurations.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_CONFIGURATIONS_H_
#define LIBS_CONFIGURATIONS_H_
//<#includes>

#include <basictypes.h>
#include <netinterface.h>
#include "headers/Web_Functions.h"
#include <headers/MicrometerProcess.h>

#define MAX_USER_PARAM_SIZE (8192)//8K
#define VERIFY_KEY (0x48666055)   // NV Settings key code

//</#includes>
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
/******************************************Horometer information***************************************/
	const char * getHMaker();
	const char * getHType();
	const char * getHSerialNum();
/******************************************Device information***************************************/
	const char * getDOperator();
	const char * getDSubStation();
	const char * getDDevice();
	const char * getDSerialNum();
	const char * getDMaker();
	const char * getDCapacity();
	const char * getDType();
	const char * getDTension();
	uint32_t getDReportNum();
	const char * getDDivision();
	const char * getDZone();
	const char * getDDate();
	const char * getDDateLastTest();
/******************************************Test information***************************************/
	ProcessState getTInitialState();
	uint8_t getTCycleNumber();
	uint8_t getTCycleWaitTime();
	uint8_t getTCmdWaitTime();
	uint16_t getTClosePulseTime();
	uint16_t getTOpenPulseTime();
	uint16_t getTPhasesWaitTime();
	uint16_t getTMaxDiffClose();
	uint16_t getTMaxDiffOpen();
	uint16_t getTMaxTimeAfterPulse();

	void setNewConfiguration(Configurations * configs);
	void getConfigurations();
	void reset();
//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_CONFIGURATIONS_H_ */
