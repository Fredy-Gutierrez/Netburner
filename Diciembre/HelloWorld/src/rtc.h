/* Revision: 3.3.0 */

/******************************************************************************
* Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/


/*******************************************************************************
 * The following is a description of the basic time structure defined in the
 * time.h standard C library, which is used by the RTC functions.
 *
 * struct tm
 * {
 *    int tm_sec;     // Second (0-59)
 *    int tm_min;     // Minute (0-59)
 *    int tm_hour;    // Hour (0-23)
 *    int tm_mday;    // Day of the month (1-31)
 *    int tm_mon;     // Month of the year [January(0)-December(11)]
 *    int tm_year;    // Years since 1900
 *    int tm_wday;    // Day of the week [Sunday(0)-Saturday(6)]
 *    int tm_yday;    // Days since January 1st (0-365)
 *    int tm_isdst;   // Daylight Saving Time flag (in effect if greater than
 *                    // zero, not in effect if equal to zero, information not
 *                    // available if less than zero)
 * };
 ******************************************************************************/

#ifndef _RTC_H
#define _RTC_H

#include <time.h>


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */



/**
 * Initialize the I2C peripheral and configure the pin functions.
 * i2cModuleNum = MODM7AE70 I2C peripheral module number: 0, 1 or 2.
 *
 * Returns 0 on success, 1 on failure.
 */
int8_t rtcInitExternalRtc(uint8_t i2cModuleNum);


/**
 * Gets the current RTC time.
 *
 * bts    - The basic time structure used to store the current time read from
 *          the RTC.
 *
 * return - '0' if successful, '1' if failure.
 */
int rtcGetTime( struct tm &bts );


/**
 * Sets the RTC time with a given time structure.
 *
 * bts    - The basic time structure used to set the RTC time.
 *
 * return - '0' if successful, '1' if failure.
 */
int rtcSetTime( struct tm &bts );


/**
 * Sets the system time with the current RTC time.
 *
 * return - '0' if successful, '1' if failure.
 */
int rtcSetSystemTimeFromRtc( void );


/**
 * Sets the RTC time with the current system time.
 *
 * return - '0' if successful, '1' if failure.
 */
int rtcSetRtcFromSystemTime( void );


void rtcSetRtcManual(int sec, int min, int hour, int mday, int mon, int year );  // , int wday, int yday);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* _RTC_H */
