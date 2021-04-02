/*
 * Rtc_Controller.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include <stdio.h>      // Include for NTP functions
#include <time.h>
#include "libs/Rtc_Controller.h"
#include "libs/I2C_Controller.h"
#include "libs/Default_Values.h"
#include "libs/Sargas_Connection.h"
#include <nbrtos.h>
#include <dns.h>
#include <tcp.h>
#include <netinterface.h>
#include <nbtime.h>

#define I2C_RTC_ADDRESS  (0xA2 >> 1)  // NXP RTC, address bits = 0x51

uint8_t registerAdd = 0x02;

/*-------------------------------------------------------------------------------------------
 * Create a BCD number from an integer
 *------------------------------------------------------------------------------------------*/
uint8_t MakeBCD(int i) {
	uint8_t bv = i % 10;
	bv |= ((i / 10) % 10) * 16;
	return bv;
}

/*-------------------------------------------------------------------------------------------
 * Return the integer value of a BCD number
 *------------------------------------------------------------------------------------------*/
int GetBCD(uint8_t bv) {
	int i = bv & 0x0F;
	i += 10 * ((bv >> 4) & 0x0f);
	return i;
}

BOOL rtcSetTime(struct tm &bts) {
	uint8_t RTCdata[30];

	RTCdata[0] = MakeBCD(bts.tm_sec);
	RTCdata[1] = MakeBCD(bts.tm_min);
	RTCdata[2] = MakeBCD(bts.tm_hour);
	RTCdata[3] = MakeBCD(bts.tm_mday);
	RTCdata[4] = bts.tm_wday;
	RTCdata[5] = MakeBCD(bts.tm_mon + 1);

	if (bts.tm_year > 99) {
		RTCdata[6] = MakeBCD(bts.tm_year - 100);
	} else {
		RTCdata[5] |= 0x80;     // set century bit
		RTCdata[6] = MakeBCD(bts.tm_year);
	}

	if (writeI2C(I2C_RTC_ADDRESS, registerAdd, RTCdata, 7)) {
		return true;   // success
	} else {
		return false;   // failure
	}
}

BOOL GetNewTime(struct tm &bts) {
	uint8_t RTCdata[30];

	if (readI2C(I2C_RTC_ADDRESS, registerAdd, RTCdata, 7)) {
		bts.tm_sec = GetBCD(RTCdata[0] & 0x7F);
		bts.tm_min = GetBCD(RTCdata[1] & 0x7F);
		bts.tm_hour = GetBCD(RTCdata[2] & 0x3F);
		bts.tm_mday = GetBCD(RTCdata[3] & 0x3F);
		bts.tm_wday = GetBCD(RTCdata[4] & 0x07);
		bts.tm_mon = (GetBCD(RTCdata[5] & 0x1F) - 1);
		if (RTCdata[5] & 0x80)
			bts.tm_year = GetBCD(RTCdata[6]);
		else
			bts.tm_year = (GetBCD(RTCdata[6]) + 100);

		return true;
	} else {
		return false;
	}
}

BOOL rtcSetSystemTimeFromRtc(void) {
	struct tm bts;
	int rv = GetNewTime(bts);

	if (rv) {
		time_t t = timegm(&bts);
		set_time(t);
		return true;
	}

	return false;
}

int rtcSetRtcFromSystemTime(void) {
	time_t t = time( NULL);
	struct tm *ptm = gmtime(&t);
	return rtcSetTime(*ptm);
}

void rtcSetManual(int sec, int min, int hour, int mon, int mday, int wday,
		int year) {

	struct tm tmNewTime;

	tmNewTime.tm_sec = sec;
	tmNewTime.tm_min = min;
	tmNewTime.tm_hour = hour;

	tmNewTime.tm_mday = mday;          // day of month
	tmNewTime.tm_mon = mon - 1;       // months since Jan
	tmNewTime.tm_wday = wday;          // days since Sunday (0-6)
	tmNewTime.tm_year = year - 1900;   // years since 1900

	rtcSetTime(tmNewTime);
}

void rtcGetTime(struct tm &bts) {
	if (GetNewTime(bts)) {

	} else {
		iprintf("Rtc fail");
	}
}

void printRtcTime() {
	char buffer[128];      // For displaying time strings*/

	/********** Display RTC Information ***********/
	iprintf("\r\n*** Real-Time Clock Time Information ***\r\n");
	struct tm stmRtc;
	rtcGetTime(stmRtc);
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S (UTC)", &stmRtc);
	iprintf("RTC Time   = %s\r\n", buffer);
	iprintf("\r\n");
}

void ntpProcess(void * pd) {
	BOOL result = FALSE;
	while (!result) {
		IPADDR ipAddress = IPADDR::NullIP();

		iprintf("Resolving NTP server name: %s...", NTP_SERVER_NAME);
		int rv = GetHostByName(NTP_SERVER_NAME, &ipAddress, INADDR_ANY, TICKS_PER_SECOND * 10);
		ipAddress.print();
		iprintf("\r\n");
		if (rv == DNS_OK) {
			iprintf("Using NTP Server to set system time\r\n");
			result = SetNTPTime(ipAddress);
			if (!result) {
				iprintf(
						"SetNTPTime() failed, waiting 30 seconds to try again\r\n");
				OSTimeDly(TICKS_PER_SECOND * 5);
			}
		} else {
			iprintf(
					"Name resolution failed, %d, waiting 5 seconds to try again\r\n",
					rv);
			OSTimeDly(TICKS_PER_SECOND * 5);
		}
	}
	iprintf("NTP success\r\n");
	rtcSetRtcFromSystemTime();
	SargasSetTimeAndDate();
}

void getHoraNtp() {
	OSSimpleTaskCreatewName(ntpProcess, MAIN_PRIO + 3, "NtpTask");
}

/*******************************************************************INIT******************************************************************/
void initRtc() {
	getHoraNtp();
}

