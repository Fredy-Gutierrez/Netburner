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

//</#include>


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
	void printSystemTime();
	void getHoraNtp();
	void rtcSetManual(int sec, int min, int hour, int mon, int mday, int wday, int year);
	BOOL GetRtcTime( struct tm &bts );
	int rtcSetRtcFromSystemTime( void );
	BOOL rtcSetSystemTimeFromRtc( void );
	void getDateIntoBuffer(char * buffer, char separateBy);
	void getTimeIntoBuffer(char * buffer, char separateBy);
	void initRtc();
//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_RTC_CONTROLLER_H_ */
