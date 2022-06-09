/*
 * Rtc_Controller.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include <stdio.h>      // Include for NTP functions
#include <time.h>
#include "headers/Rtc_Controller.h"
#include "headers/I2C_Controller.h"
#include "headers/Default_Values.h"
#include "headers/Configurations.h"
#include <nbrtos.h>
#include <dns.h>
#include <tcp.h>
#include <netinterface.h>
#include <nbtime.h>

#define I2C_RTC_ADDRESS  (0xA2 >> 1)  // NXP RTC, address bits = 0x51

uint8_t registerAdd = 0x01;//start register address, 02=Development board, 01=UTR Aei

/*-------------------------------------------------------------------------------------------
 * Create a BCD number from an integer
 *------------------------------------------------------------------------------------------*/
uint8_t MakeBCD( int i )
{
   uint8_t bv = i % 10;
   bv |= ( ( i / 10 ) % 10 ) * 16;
   return bv;
}

/*-------------------------------------------------------------------------------------------
 * Return the integer value of a BCD number
 *------------------------------------------------------------------------------------------*/
int GetBCD( uint8_t bv )
{
   int i = bv & 0x0F;
   i += 10 * ( ( bv >> 4 ) & 0x0f );
   return i;
}


/**************************************SETS THE RTC TIME USING THE I2C PROTOCOL******************************************/
BOOL rtcSetTime(struct tm &bts){
	uint8_t RTCdata[30];

    RTCdata[0] = MakeBCD(bts.tm_sec);
    RTCdata[1] = MakeBCD(bts.tm_min);
    RTCdata[2] = MakeBCD(bts.tm_hour);
    RTCdata[3] = MakeBCD(bts.tm_mday);
    RTCdata[4] = bts.tm_wday;
    RTCdata[5] = MakeBCD(bts.tm_mon + 1);

    if (bts.tm_year > 99)
    {
        RTCdata[6] = MakeBCD(bts.tm_year - 100);
    }
    else
    {
        RTCdata[5] |= 0x80;     // set century bit
        RTCdata[6] = MakeBCD(bts.tm_year);
    }

    if ( writeI2C(I2C_RTC_ADDRESS,registerAdd,RTCdata,7) )
    {
    	return true;   // success
    }
    else
    {
    	return false;   // failure
    }
}


/**************************************GETS THE RTC TIME USING THE I2C PROTOCOL******************************************/
BOOL GetRtcTime( struct tm &bts )
{
	uint8_t RTCdata[30];

    if ( readI2C(I2C_RTC_ADDRESS, registerAdd, RTCdata, 7) )
    {
        bts.tm_sec =  GetBCD(RTCdata[0] & 0x7F);
        bts.tm_min =  GetBCD(RTCdata[1] & 0x7F);
        bts.tm_hour = GetBCD(RTCdata[2] & 0x3F);
        bts.tm_mday = GetBCD(RTCdata[3] & 0x3F);
        bts.tm_wday = GetBCD(RTCdata[4] & 0x07);
        bts.tm_mon = (GetBCD(RTCdata[5] & 0x1F) - 1);
        if (RTCdata[5] & 0x80)
            bts.tm_year = GetBCD(RTCdata[6]);
        else
            bts.tm_year = (GetBCD(RTCdata[6]) + 100);

        return true;
    }
    else
    {
    	return false;
    }
}


/**************************************SETS EXTERNAL RTC TIME TO THE SYTEM TIME******************************************/
BOOL rtcSetSystemTimeFromRtc( void )
{
   struct tm bts;
   int rv = GetRtcTime( bts );

   if ( rv )
   {
	   time_t t = timegm( &bts );
	   set_time( t );
	   return true;
   }

   return false;
}


/**************************************SETS THE SYTEM TIME TO EXTERNAL RTC TIME******************************************/
int rtcSetRtcFromSystemTime( void )
{
   time_t t = time( NULL );
   struct tm *ptm = gmtime( &t );
   return rtcSetTime( *ptm );
}


/**************************************SETS THE TIME (MANUAL) TO THE EXTERNAL RTC******************************************/
void rtcSetManual(int sec, int min, int hour, int mon, int mday, int wday, int year){

	struct tm tmNewTime;

	tmNewTime.tm_sec  = sec;
	tmNewTime.tm_min  = min;
	tmNewTime.tm_hour = hour;

	tmNewTime.tm_mday = mday;          // day of month
	tmNewTime.tm_mon  = mon - 1;       // months since Jan
	tmNewTime.tm_wday = wday;          // days since Sunday (0-6)
	tmNewTime.tm_year = year - 1900;   // years since 1900

	rtcSetTime(tmNewTime);
	rtcSetSystemTimeFromRtc();
}

void getDateIntoBuffer(char * buffer, char separateBy){
	time_t t = time( NULL );
	struct tm *ptm = gmtime( &t );
	//struct tm *ptm = localtime( &t );
	sprintf(buffer, "%04hd%c%02hd%c%02hd", (short)ptm->tm_year + 1900,separateBy, (short)ptm->tm_mon + 1,separateBy , (short)ptm->tm_mday);
}

void getTimeIntoBuffer(char * buffer, char separateBy){
	time_t t = time( NULL );
	//struct tm *ptm = localtime( &t );
	struct tm *ptm = gmtime( &t );
	sprintf(buffer, "%02hd%c%02hd%c%02hd", (short)ptm->tm_hour,separateBy, (short)ptm->tm_min,separateBy , (short)ptm->tm_sec);
}

/**************************************NTP FUNTION, IT GET THE NTP HOUR AND SAVE IT IN SYSTEM TIME******************************************/
uint8_t stateSynch;
void ntpProcess(void * pd){
	int retriesCounter=0;
	int rv = 0;
	IPADDR ipAddress=IPADDR::NullIP();
	bool ntpTask = true;
	while(ntpTask){
		switch(stateSynch){
			case 0:
				retriesCounter=0;
				stateSynch=1;
				break;
			case 1:
				rv=GetHostByName(DEFAULT_NTP_SERVER_NAME, &ipAddress, INADDR_ANY, TICKS_PER_SECOND * 10);
				if( rv == DNS_OK ){
					bool result;
					result=SetNTPTime(ipAddress);
					if(!result){
						if(++retriesCounter >= DEFAULT_NTP_RETRIES){
							ntpTask = false;
						}
					}else{
						tzsetchar((char*)"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00");
						stateSynch=2;
					}
				}else{
					iprintf("Name resolution failed, %d, waiting 5 seconds to try again\n", rv);
					if(++retriesCounter >= DEFAULT_NTP_RETRIES){
						ntpTask = false;
					}
					OSTimeDly(TICKS_PER_SECOND * 2);
				}
				break;
			case 2:
				time_t t = time( NULL );
				struct tm *ptm = localtime( &t );
				rtcSetManual(ptm->tm_sec, ptm->tm_min, ptm->tm_hour, ptm->tm_mon+1, ptm->tm_mday, ptm->tm_wday, ptm->tm_year+1900);
				//rtcSetRtcFromSystemTime();//saves the ntp hour in external rtc
				iprintf("NTP Time used\n");
				ntpTask = false;
				break;
		}
	}
}


/*******************************************************************START THE RTC AND GET THE NTP HOUR******************************************************************/
void initRtc(){
	rtcSetSystemTimeFromRtc();
	stateSynch=0;
	OSSimpleTaskCreatewName(ntpProcess,NTP_TASK_PRIO,"NtpTask");
}

