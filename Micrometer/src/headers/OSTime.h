/*
 * OSTime.h
 *
 *  Created on: 21 sep 2021
 *      Author: Integra Fredy
 */

#ifndef HEADERS_OSTIME_H_
#define HEADERS_OSTIME_H_


//<#include>
#include <time.h>
#include <basictypes.h>
#include <hal.h>

//</#include>

typedef struct {
	unsigned long long Secs;
	uint32_t msSec;
	uint32_t uSec;
}IOTime;

enum TimeStatus { initStatus,gettingTimeStatus, stopStatus };

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
	/*void setSystemTime(typeAeiTime * time);
	void convertTimeToASCII(typeAeiTime * t, char * asciiFormat);*/
	uint32_t getMsTime();
	const char * getUsTime();
	void updateUsec();
	void getIOTimeDiference(IOTime * timeInit, IOTime * timeEnd, IOTime * destination);
	void getElapseTime(volatile IOTime * t);
	/*void getNowTimeAscii(char * tNowAscii);
	void copyAeiTime(typeAeiTime * destT, typeAeiTime * origT);
	int cmpAeiTime(typeAeiTime * t1, typeAeiTime * t2); // return 1 si t1>t2, -1 si t2>t1 y 0 si son iguales
	int difAeiTime(typeAeiTime * t1, typeAeiTime * t2);
	void addAeiTime(typeAeiTime * t, int milisecs);
	void getSargasTime(char * msg, typeAeiTime *t, int index1, int index2);
	int getHourOfDay();*/
	void stopGettingTime();
	void startGettingTime();
	void initTimer();


//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HEADERS_OSTIME_H_ */
