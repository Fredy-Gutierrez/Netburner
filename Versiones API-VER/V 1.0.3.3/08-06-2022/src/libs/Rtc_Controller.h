/*
 * Rtc_Controller.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_RTC_CONTROLLER_H_
#define LIBS_RTC_CONTROLLER_H_

//<#include>
#include <time.h>
#include <basictypes.h>

typedef struct {
	time_t sysTime;
	uint32_t msCurrSec;
}typeAeiTime;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
	void printSystemTime();
	void getHoraNtp();
	void initRtc();
	void rtcSetManual(int sec, int min, int hour, int mon, int mday, int wday, int year);
	BOOL GetRtcTime( struct tm &bts );
	int rtcSetRtcFromSystemTime( void );
	BOOL rtcSetSystemTimeFromRtc( void );

	void setSystemTime(typeAeiTime * time);
	void convertTimeToASCII(typeAeiTime * t, char * asciiFormat);
	uint32_t getMsTime();
	const char * getUsTime();
	void updateUsec(register uint32_t chgSec);
	void getNowTime(typeAeiTime * t);
	void getNowTimeAscii(char * tNowAscii);
	void copyAeiTime(typeAeiTime * destT, typeAeiTime * origT);
	int cmpAeiTime(typeAeiTime * t1, typeAeiTime * t2); // return 1 si t1>t2, -1 si t2>t1 y 0 si son iguales
	int difAeiTime(typeAeiTime * t1, typeAeiTime * t2);
	void addAeiTime(typeAeiTime * t, int milisecs);
	void getSargasTime(char * msg, typeAeiTime *t, int index1, int index2);
	int getHourOfDay();

	void getDateIntoBuffer(char * buffer, char separateBy);
	void addDaysTo(char * buffer, char * newBuffer, int days);
	int compareDates(char * date1, char * date2);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_RTC_CONTROLLER_H_ */
