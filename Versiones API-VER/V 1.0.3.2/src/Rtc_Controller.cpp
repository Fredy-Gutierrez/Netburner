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
#include "libs/SargasProcess.h"
#include "libs/Log.h"
#include "libs/Configurations.h"
#include <nbrtos.h>
#include <dns.h>
#include <tcp.h>
#include <netinterface.h>
#include <nbtime.h>
#include <stdlib.h>

extern logger logg;

#define I2C_RTC_ADDRESS  (0xA2 >> 1)  // NXP RTC, address bits = 0x51

uint8_t registerAdd = 0x01;//start register address, 02=Development board, 01=UTR Aei
volatile uint32_t msCurrSec = 0;		//milisecs of current second
volatile uint32_t usCurrSec = 0;		//microsecs of current second, to get current value, multiplie by 200

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

/**************************************PRINT THE CURRENT TIME IN THE EXTERNAL RTC******************************************/
void printSystemTime(){
	char buffer[50];      // For displaying time strings*/

	/********** Display System Time ***********/
	getNowTimeAscii(buffer);
	logg.newPrint("\nSystem Time = %s", buffer);
}


extern void updateSychStatus(bool _synch);
int stateSynch;
/**************************************NTP FUNTION, IT GET THE NTP HOUR AND SAVE IT IN SYSTEM TIME******************************************/
void ntpProcess(void * pd){
	int tickCounter=0, retriesCounter=0;
	int periods=0;
	int rv=0;
	IPADDR ipAddress=IPADDR::NullIP();

	while(1){
		switch(stateSynch){
			case 0:
				OSTimeDly(TICKS_PER_SECOND * 20);
				retriesCounter=0;
				stateSynch=1;
			break;

			case 1:
				rv=GetHostByName(DEFAULT_NTP_SERVER_NAME, &ipAddress, INADDR_ANY, TICKS_PER_SECOND * 10);
				if(rv==DNS_OK){
					bool result;
					result=SetNTPTime(ipAddress);
					if(!result){
						logg.newPrint("\nSetNTPTime() failed, waiting 5 minutes to try again");
						if(++retriesCounter>=12){
							updateSychStatus(FALSE);
							tickCounter=0;
							stateSynch=2;
						}
					}
					else{
						updateSychStatus(TRUE);
						tickCounter=0;
						rtcSetRtcFromSystemTime();
						stateSynch=2;
					}
				}
				else{
					logg.newPrint("\nName resolution failed, %d, waiting 5 minutes to try again", rv);
					if(++retriesCounter>=12){
						updateSychStatus(FALSE);
						tickCounter=0;
						stateSynch=2;
					}
				}
				OSTimeDly(TICKS_PER_SECOND * 300);
			break;

			case 2:
				periods=getNtpSyncTime()*12;
				if(++tickCounter>=periods){
					retriesCounter=0;
					stateSynch=1;
				}
				OSTimeDly(TICKS_PER_SECOND * 300);
			break;

		}
	}
}


/*******************************************************************START THE RTC AND GET THE NTP HOUR******************************************************************/
void initRtc(){
	rtcSetSystemTimeFromRtc();
	if(getNtpSyncOnOff()==TRUE){
		stateSynch=0;
		OSSimpleTaskCreatewName(ntpProcess,NTP_TASK_PRIO,"NtpTask");
	}
}

/*********************************************** CONVERTS time_t TO ASCII FORMAT ******************************************************************/
/*                                                 AAAA-MM-DDThh:mm:ss.mil              */
void convertTimeToASCII(typeAeiTime * t, char * asciiFormat) {
	char temp[20];
	struct tm *ptm = gmtime( & t->sysTime );

	sprintf(asciiFormat, "%4d-", ptm->tm_year+1900);

	sprintf(temp, "%02d-", ptm->tm_mon+1);
	strcat(asciiFormat, temp);
	sprintf(temp, "%02dT", ptm->tm_mday);
	strcat(asciiFormat, temp);
	sprintf(temp, "%02d:", ptm->tm_hour);
	strcat(asciiFormat, temp);
	sprintf(temp, "%02d:", ptm->tm_min);
	strcat(asciiFormat, temp);
	sprintf(temp, "%02d.", ptm->tm_sec);
	strcat(asciiFormat, temp);
	sprintf(temp, "%03d", (int)t->msCurrSec);
	strcat(asciiFormat, temp);

}

int getHourOfDay() {
	time_t sysTime=time(0);
	struct tm *ptm = gmtime( & sysTime );

	return (int)ptm->tm_hour;

}


/*******************************************************************RETURNS CURRENT SYSTEM TIME AND MILISECS******************************************************************/
uint32_t getMsTime(){
	return(msCurrSec);
}

const char * getUsTime(){
	char * currUsec="                ";
	sprintf(currUsec, "%llu", ((unsigned long long)time(0))*1000+(unsigned long long)msCurrSec);
	return (const char *)currUsec;
}

void setSystemTime(typeAeiTime * time){
	set_time(time->sysTime);
	msCurrSec=time->msCurrSec;
}

void getNowTime(typeAeiTime * tNow){
	tNow->sysTime=time(0);
	tNow->msCurrSec=msCurrSec;
}

void getNowTimeAscii(char * tNowAscii){
	typeAeiTime tNow;
	tNow.sysTime=time(0);
	tNow.msCurrSec=msCurrSec;
	convertTimeToASCII(& tNow, tNowAscii);
}

void copyAeiTime(typeAeiTime * destT, typeAeiTime * origT){
	destT->sysTime=origT->sysTime;
	destT->msCurrSec=origT->msCurrSec;
}

// return 1 si t1>t2, -1 si t2>t1 y 0 si son iguales
int cmpAeiTime(typeAeiTime * t1, typeAeiTime * t2){
	if(t1->sysTime>t2->sysTime)
		return 1;
	else if(t2->sysTime>t1->sysTime)
		return -1;

	if(t1->msCurrSec>t2->msCurrSec)
		return 1;
	else if(t2->msCurrSec>t1->msCurrSec)
		return -1;

	return 0;
}

void updateUsec(register uint32_t chgSec){
	if(chgSec == 1){
		msCurrSec = 0;
		usCurrSec = 0;
	}
	else{
		if(++usCurrSec == 5){
			msCurrSec++;
			usCurrSec = 0;
		}
	}
}

//Adds milisecs to the indicated time
void addAeiTime(typeAeiTime * t, int milisecs){
	unsigned long long absTime;

	absTime=(unsigned long long)t->sysTime*1000+(unsigned long long)t->msCurrSec;
	if(milisecs>=0){
		absTime+=(unsigned long long)milisecs;
	}
	else{
		absTime-=(unsigned long long)(milisecs*(-1));
	}
	t->msCurrSec=(uint32_t)(absTime%1000);
	t->sysTime=(time_t)((absTime/1000));
}


//Calculates milisecs difference
int difAeiTime(typeAeiTime * t1, typeAeiTime * t2){
	int diff;

	diff=((int)t2->sysTime-(int)t1->sysTime)*1000 + ((int)t2->msCurrSec- (int)t1->msCurrSec);

	return diff < 0 ? diff * -1 : diff;
}

void getSargasTime(char * msg, typeAeiTime *t, int index1, int index2){
    time_t ttYears = 0;     // Seconds converted from tm_year (since 1900)
    time_t ttMonths = 0;    // Seconds converted from tm_mon (0-11)
    time_t ttDays = 0;      // Seconds converted from tm_mday (1-31)
    time_t ttHours = 0;     // Seconds converted from tm_hour (0-23)
    time_t ttMinutes = 0;   // Seconds converted from tm_min (0-59)
    time_t ttSecs = 0;      // Cumulative seconds including tm_sec (0-59)

    int daysPerMonth[] = {31 /* Jan */, 28 /* Feb */, 31 /* Mar */, 30 /* Apr */, 31 /* May */, 30 /* Jun */,
                          31 /* Jul */, 31 /* Aug */, 30 /* Sep */, 31 /* Oct */, 30 /* Nov */, 31 /* Dec */};

    int tm_year;
    tm_year=((int)msg[index2+18]-0x30)*10+(int)msg[index2+19]-0x30+30; //0 is equivalent to 2000, must be converted to 70 to be equivalent to 1970
    // Extract time_t seconds from tm_year (time_t starts from 1970-01-01)
    ttYears = tm_year * 365 * 24 * 3600;

    int tm_month=((int)msg[index2+12]-0x30)*10+(int)msg[index2+13]-0x30-1;
    // Extract time_t seconds from tm_mon
    for (int i = 0; i < tm_month; i++){
        ttMonths += daysPerMonth[i];
    }
    ttMonths *= 24 * 3600;

    int aux=(msg[index2+15]-0x30)*10+msg[index2+16]-0x30-1;
    // Extract time_t seconds from tm_mday
    ttDays = aux * 24 * 3600;
    ttDays += (((tm_year +1)) / 4) * 24 * 3600;   // Add leap days excluding current year

    aux=(msg[index1+0]-0x30)*10+msg[index1+1]-0x30;
    // Extract time_t seconds from tm_hour
    ttHours = aux * 3600;

    aux=(msg[index1+3]-0x30)*10+msg[index1+4]-0x30;
    // Extract time_t seconds from tm_min
    ttMinutes = aux * 60;

    aux=(msg[index1+6]-0x30)*10+msg[index1+7]-0x30;
    // Tally up extracted seconds including tm_sec
    ttSecs = aux + ttMinutes + ttHours + ttDays + ttMonths + ttYears;

    // Determine if current leap year and add add leap day if necessary
    if (((tm_year % 4) == 0) && (tm_month >= 1)) { ttSecs += 24 * 3600; }

    aux=(msg[index1+9]-0x30)*100+(msg[index1+10]-0x30)*10;
    if(index1!=index2)
    	aux+=msg[index1+11]-0x30;

    t->sysTime=ttSecs;
    t->msCurrSec=(uint32_t)aux;
}

void getDateIntoBuffer(char * buffer, char separateBy){
	time_t t = time( NULL );
	struct tm *ptm = gmtime( &t );
	//ddMMyyyy
	if(separateBy != '\0'){
		sprintf(buffer, "%02hd%c%02hd%c%04hd",(short)ptm->tm_mday, separateBy , (short)ptm->tm_mon + 1,separateBy , (short)ptm->tm_year + 1900);
	}else{
		sprintf(buffer, "%02hd%02hd%04hd",(short)ptm->tm_mday , (short)ptm->tm_mon + 1 , (short)ptm->tm_year + 1900);
	}
}

//the buffer format must be ddMMyyyy
void makeTmFrom(char * buffer, struct tm * timeinfo){
	char tempBuff[5];
	memset(tempBuff, '\0',sizeof(tempBuff));

	memcpy(tempBuff,buffer+2,2);
	tempBuff[2] = '\0';
	int month = atoi(tempBuff);

	memcpy(tempBuff,buffer,2);
	tempBuff[2] = '\0';
	int day = atoi(tempBuff);

	memcpy(tempBuff,buffer+4,4);
	tempBuff[4] = '\0';
	int year = atoi(tempBuff);

	timeinfo->tm_year = year - 1900;
	timeinfo->tm_mon = month - 1;
	timeinfo->tm_mday = day;
}

void addDaysTo(char * buffer, char * newBuffer, int days){
	time_t rawtime;
	struct tm * timeinfo;
	timeinfo = localtime( &rawtime );

	makeTmFrom(buffer, timeinfo);

	timeinfo->tm_mday += days;

	mktime ( timeinfo );
	sprintf(newBuffer, "%02hd%02hd%04hd",(short)timeinfo->tm_mday , (short)timeinfo->tm_mon + 1 , (short)timeinfo->tm_year + 1900);
}

/***********************COMPARE TWO DATES******************************
 * the date format must be ddMMyyyy
 *
 * 		@date1 		->		buffer which contains first date
 * 		@date2 		->		buffer which contains second date
 *
 * 		return 				1 for date1 > date2
 * 							0 for date1 = date2
 * 						   -1 for date1 < date2
 * */
int compareDates(char * date1, char * date2){
	time_t seconds = time(NULL);
	struct tm *timeinfo = localtime(&seconds);

	struct tm start = *timeinfo;
	struct tm end = *timeinfo;

	makeTmFrom(date1, &start);
	makeTmFrom(date2, &end);

	double d = difftime( mktime(&start), mktime(&end));
	if(d == 0){
		return 0;
	}else if(d > 0){
		return 1;
	}else{
		return -1;
	}
}



