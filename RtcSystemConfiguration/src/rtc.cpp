/*NB_REVISION*/

/*NB_COPYRIGHT*/

#include <predef.h>
#include <time.h>
#include <constants.h>
#include <basictypes.h>
#include <system.h>
#include <nbrtos.h>
#include <i2c.h>
#include <nbtime.h>
#include <pins.h>
#include "rtc.h"



// Specify the I2C peripheral module and RTC device
#define I2C_MODULE_NUM    0           // use I2C0/TW0
#define I2C_RTC_ADDRESS  (0xA2 >> 1)  // NXP RTC, address bits = 0x51
#define I2C_BUS_SPEED    (100000)     // Bus speed of 100kHz

//  Create an I2C device object for the RTC
I2CDevice RTCDevice(i2c[I2C_MODULE_NUM], I2C_RTC_ADDRESS);

uint8_t RTCdata[30];

/*------------------------------------------------------------------------------
 * Initialize the I2C peripheral and configure the pin functions.
 * i2cModuleNum = MODM7AE70 I2C peripheral module number: 0, 1 or 2.
 *
 * Returns 0 on success, 1 on failure.
 *-----------------------------------------------------------------------------*/
int8_t rtcInitExternalRtc(uint8_t i2cModuleNum)
{
    int8_t status = 0;

    if( (i2cModuleNum < 0) || (i2cModuleNum > 2) )
    {
        status = 1;
    }
    else
    {

        switch(i2cModuleNum)
        {
            case 0:
                P2[39].function(PINP2_39_TWD0);
                P2[42].function(PINP2_42_TWCK0);
                i2c[0].setup(I2C_BUS_SPEED);
                break;
            case 1:
                P2[22].function(PINP2_22_TWD1);
                //P2[12].function(PINP2_12_TWCK1);
                i2c[1].setup(I2C_BUS_SPEED);
                break;
            case 2:
                P2[26].function(PINP2_26_TWD2);
                P2[23].function(PINP2_23_TWCK2);
                i2c[2].setup(I2C_BUS_SPEED);
                break;
            default:
                status = 1;
                //iprintf("invalid moduleNum\r\n");
                break;
        }
    }

    return status;
}



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



/*-------------------------------------------------------------------------------------------
 * Get the time from the NXP PCF8563 RTC. The time is stored in tm structure pointed to
 * by bts.
 *
 * The time values in the RTC are stored in seven 8-bit registers, 0x02 - 0x08 in BCD format.
 *
 * Returns 0 on success, 1 on failure.
 *------------------------------------------------------------------------------------------*/
int rtcGetTime_NXPPCF8563( struct tm &bts )
{
    int status = 0;

    if ( RTCDevice.readRegN(0x02, RTCdata, 7) == I2C::I2C_RES_ACK)
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

        status = 0;
    }
    else
    {
        status = 1;
    }

    return status;
}


/*-------------------------------------------------------------------------------------------
 * Set the RTC time in the  NXP PCF8563. Writes the calendar time to the RTC registers.
 *
 * Returns 0 for success, 1 for failure.
 *------------------------------------------------------------------------------------------*/
int rtcSetTime_NXPPCF8563( struct tm &bts )
{
    int status = 0;

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

    if ( RTCDevice.writeRegN(0x02, RTCdata, 7) == I2C::I2C_RES_ACK)
    {
        status = 0;   // success
    }
    else
    {
        status = 1;   // failure
    }
    return status;
}


/*-------------------------------------------------------------------------------------------
 * Get RTC time. Writes the values of the calendar time tm structure referenced by bts.
 *
 * Returns 0 for success, 1 for failure.
 *------------------------------------------------------------------------------------------*/
int rtcGetTime( struct tm &bts )
{
    return rtcGetTime_NXPPCF8563(bts);
}


/*-------------------------------------------------------------------------------------------
 * Abstracted set RTC time function with the values referenced by the bts parameter.
 *
 * Returns 0 for success, 1 for failure.
 *------------------------------------------------------------------------------------------*/
int rtcSetTime( struct tm &bts )
{
    return rtcSetTime_NXPPCF8563(bts);
}


/*-------------------------------------------------------------------------------------------
 * Set the main system time from the RTC time
 *
 * Returns 0 on success, 1 on failure.
 *------------------------------------------------------------------------------------------*/
int rtcSetSystemTimeFromRtc( void )
{
   struct tm bts;
   int rv = rtcGetTime( bts );

   if ( rv )
   {
      return rv;
   }

   time_t t = timegm( &bts );
   set_time( t );
   return 0;
}

/*-------------------------------------------------------------------------------------------
 * Set the RTC time from the main system time
 *
 * Returns 0 on success, 1 on failure.
 *------------------------------------------------------------------------------------------*/
int rtcSetRtcFromSystemTime( void )
{
   time_t t = time( NULL );
   struct tm *ptm = gmtime( &t );
   return rtcSetTime( *ptm );
}

/*-------------------------------------------------------------------
 * Manually set the RTC time.
 *

   typedef struct tm {
      int tm_sec;    // seconds after the minute (0 - 59)
      int tm_min;    // minutes after the hour (0 - 59)
      int tm_hour;   // hours since midnight (0 - 23)
      int tm_mday;   // day of month (1 - 31)
      int tm_mon;    // months since January (0 - 11 )
      int tm_year;   // Years since 1900
      int tm_wday;   // Days since Sunday (0 - 6, 0 = Sunday)
      int tm_yday;   // Days since January 1 (0 - 365)
      int tm_isdst;  // daylight savings time flag
   }

 -------------------------------------------------------------------*/
void rtcSetRtcManual(int hour, int min, int sec, int mon, int mday, int year )  //, int wday, int yday)
{
    struct tm tmNewTime;

    tmNewTime.tm_sec  = sec;
    tmNewTime.tm_min  = min;
    tmNewTime.tm_hour = hour;

    tmNewTime.tm_mday = mday;          // day of month
    tmNewTime.tm_mon  = mon - 1;       // months since Jan
    tmNewTime.tm_year = year - 1900;   // years since 1900
    //tmNewTime.tm_wday = wday;          // days since Sunday (0-6)
    //tmNewTime.tm_yday = yday;          // days since Jan 1 (0-365)
    //tmNewTime.tm_isdst = -1;           // daylight savings time 0 = no, > 0 = yes

    rtcSetTime(tmNewTime);
}



