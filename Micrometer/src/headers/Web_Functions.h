/*
 * Web_Functions.h
 *
 *  Created on: 4 oct 2021
 *      Author: Integra Fredy
 */

#ifndef HEADERS_WEB_FUNCTIONS_H_
#define HEADERS_WEB_FUNCTIONS_H_

typedef struct{
	/*******************Horometer information***************/
	char hMaker[100];
	char hType[50];
	char hSerialNum[50];
	/*******************Device to be tasted***************/
	char dOperator[100];
	char dSubStation[100];
	char dDevice[100];
	char dSerialNum[50];
	char dMaker[100];
	char dCapacity[50];
	char dType[50];
	char dTension[50];
	uint32_t dReportNum;
	char dDivision[50];
	char dZone[100];
	char dDate[15];
	char dDateLastTest[15];
	/*******************Test information***************/
	uint8_t tInitialState;
	uint8_t tCycleNumber;
	uint8_t tCycleWaitTime;
	uint8_t tCmdWaitTime;
	uint16_t tClosePulseTime;
	uint16_t tOpenPulseTime;
	uint16_t tPhasesWaitTime;
	uint16_t tMaxDiffClose;
	uint16_t tMaxDiffOpen;
	uint16_t tMaxTimeAfterPulse;
}Configurations;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */




#endif /* HEADERS_WEB_FUNCTIONS_H_ */
