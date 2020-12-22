#include <predef.h>
#include <hal.h>        // For ForceReboot() function
#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <nettypes.h>
#include <nbtime.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <i2c.h>
#include <pins.h>
#include <timezones.h>
#include "rtc.h"

const char *AppName = "External RTC Example";

int syncSystemTimeWithNTP()
{
    // Note the true parameter is optional - it sends status messages to stdout
    if (SetTimeNTPFromPool(true))
    {
        iprintf("\r\nSTATUS -- System time sync with NTP server successful\r\n");
        return 0;
    }
    else
    {
        iprintf("\r\nERROR  -- System time sync with NTP server failed\r\n");
        return -1;
    }
}

/*-----------------------------------------------------------------------------------
 * Synchronize the real-time clock with the current system time
 * Returns 0 on success, -1 on failure.
 *-----------------------------------------------------------------------------------*/
int syncRtcWithSystemTime()
{
    if (rtcSetRtcFromSystemTime() == 0)
    {
        iprintf("\r\nSTATUS -- RTC sync with system successful\r\n");
        return 0;
    }
    else
    {
        iprintf("\r\nERROR  -- RTC sync with system failed\r\n");
        return -1;
    }
}

/*-----------------------------------------------------------------------------------
 * Synchronize the system time with the current real-time clock time
 *
 * Returns 0 on success, 1 on failure.
 *-----------------------------------------------------------------------------------*/
int syncSystemTimeWithRtc()
{
    if (rtcSetSystemTimeFromRtc() == 0)
    {
        iprintf("\r\nSTATUS -- System sync with RTC successful\r\n");
        return 0;
    }
    else
    {
        iprintf("\r\nERROR  -- System sync with RTC failed\r\n");
        return -1;
    }
}

/*-----------------------------------------------------------------------------------
 * Check for synchronization between system time and RTC time.
 * The range parameter specifies the absolute value of the acceptable range for
 * comparison in seconds.
 *
 * Returns true if in sync, false if not.
 *-----------------------------------------------------------------------------------*/
bool checkSystemAndRtcTimeSync(double rangeSeconds)
{
    time_t ttSys;       // System time in time_t format
    struct tm tmRTC;    // RTC time in struct tm format
    bool status = false;

    if (rtcGetTime(tmRTC) == 0)             // Read the RTC time and store in tmRTC
    {
        ttSys = time(NULL);                 // Read the current system time
        time_t tt_rtc = timegm(&tmRTC);     // Convert RTC time from tm to time_t

        double timeDiff = difftime(ttSys, tt_rtc);
        printf("ttSys: %lld, ttRTC: %lld, diff: %0.2f\r\n", ttSys, tt_rtc, timeDiff);

        if ( fabs(timeDiff) > rangeSeconds )
            status = false;
        else
            status = true;
    }
    else
    {
        status = false;
    }

    return status;
}

/*------------------------------------------------------------------------------------------
 * Display the current system time and external RTC time
 *-----------------------------------------------------------------------------------------*/
void displaySystemAndRtcTime()
{
    iprintf("\r\n*** System Time Information ***\r\n");

    time_t timeT;          // Stores the system time in time_t format
    struct tm stmUTC;      // Stores UTC time in struct tm format
    struct tm stmLocal;    // Stores local time as struct tm
    char buffer[128];      // For displaying time strings

    timeT = time(NULL);   // Get system time as time_t

    /* Converts given time since epoch (a time_t value pointed to by timeT) into calendar time,
     * expressed in Coordinated Universal Time (UTC) in the struct tm format. The result is
     * stored in the struct tm pointed to by stmUTC.
     */
    gmtime_r(&timeT, &stmUTC);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S", &stmUTC);
    iprintf("UTC Time   = %s\r\n", buffer);

    /* Converts given time since epoch (a time_t value pointed to by timeT) into calendar time,
     * expressed in local time, in the struct tm format. The result is stored in the struct tm
     * pointed to by stmLocal.
     */
    localtime_r(&timeT, &stmLocal);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z [UTC offset: %z]", &stmLocal);
    iprintf("Local Time = %s\r\n", buffer);


    /********** Display RTC Information ***********/
    iprintf("\r\n*** Real-Time Clock Time Information ***\r\n");
    struct tm stmRtc;
    rtcGetTime(stmRtc);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S (UTC time)", &stmRtc);
    iprintf("RTC Time   = %s\r\n", buffer);

    iprintf("\r\n");
}


/*----------------------------------------------------------------------------
 * Sets the system time zone to the standard time value specified by "name".
 * Daylight savings will be automatically calculated.
 *
 * struct TimeZoneRecord
 * {
 *    const char *Posix;            // String used by tzsetchar()
 *    const char *Description;      // Full description including GMT offset
 *    const char *Name;             // Name of time zone
 *    const char *Short_name;       // Abbreviation of time zone
 *    int bUsCanada;                // Canada flag
 * };
 *
 * Returns 0 on success, -1 on failure.
 *----------------------------------------------------------------------------*/
int setTimeZoneFromName(const char *name)
{
    bool found = false;
    int i = 0;

    while ( (TZRecords[i].Short_name != NULL) && (!found) )
    {
        if ( strstr(TZRecords[i].Short_name, name) != NULL )
        {
            found = true;
        }
        else
        {
            i++;
        }
    }

    if ( found )
    {
        // set time zone
        tzsetchar((char *) TZRecords[i].Posix);

        // remove any trailing white space from short name
        char buf[16];
        int j = 0;
        while ( isalpha(TZRecords[i].Short_name[j] ) && (j < 5) )
        {
            buf[j] = TZRecords[i].Short_name[j];
            j++;
        }
        buf[j] = 0;

        iprintf("Time zone set to: [%s], %s\r\n", buf, TZRecords[i].Name);
        return 0;
    }
    else
    {
        iprintf("*** Error, time zone \"%s\" not found, index = %d\r\n", TZRecords[i].Name, i);
        return -1;
    }
}



/*----------------------------------------------------------------------------
 * Software-resets the device. This feature is useful in verifying whether the
 * RTC can retain set time information
 *----------------------------------------------------------------------------*/
void rebootDevice()
{
    iprintf("\r\nSTATUS -- Rebooting the device...\r\n");
    OSTimeDly(1);
    ForceReboot();
}

/*----------------------------------------------------------------------------
 * Displays the menu of numeric options made available by this application
 *----------------------------------------------------------------------------*/
void displayMenu()
{
    iprintf("\r\n***** NTP-System-RTC Demo Main Menu *****\r\n");
    iprintf("[1] Display current system and RTC times\r\n");
    iprintf("[2] Set the system time with NTP (Internet access required)\r\n");
    iprintf("[3] Set the RTC time to the system time\r\n");
    iprintf("[4] Set the system time to the RTC time\r\n");
    iprintf("[5] Verify synchronization between system and RTC\r\n");
    iprintf("[6] Set the RTC time manually to Jan 1, 2020, 00:00:00\r\n");
    iprintf("[0] Software-reset the device\r\n");
}

/*------------------------------------------------------------------------------
 * Serial menu command processor
 *------------------------------------------------------------------------------*/
void processCommand(char command)
{
    iprintf("\r\n");

    switch (command)
    {
        case '1': displaySystemAndRtcTime(); break;
        case '2': syncSystemTimeWithNTP(); break;
        case '3': syncRtcWithSystemTime(); break;
        case '4': syncSystemTimeWithRtc(); break;
        case '5':
            if ( checkSystemAndRtcTimeSync(5.0) )
                printf("System and RTC time are in sync\r\n");
            else
                printf("System and RTC time are NOT in sync\r\n");
            break;
        case '6':
            iprintf("Setting RTC time to Jan 1, 2020, 00:00:00\r\n");
            rtcSetRtcManual(11, 49, 0, 12, 21, 2020);
            break;
        case '0': rebootDevice(); break;
        default : displayMenu();
    }
}

void UserMain(void * pd)
{
	init();

	    iprintf("Starting NTP External RTC Example\r\n");

	    const int i2cPeripheralModuleNum = 0;   // 0, 1 or 2
	    rtcInitExternalRtc(i2cPeripheralModuleNum);

	    // Set time zone from system list in timezone.h and timezone.cpp
	    // Specify standard time abbreviation, daylight savings calculated automatically
	    setTimeZoneFromName("PST");

	    // If you want to create your own time zone, create a string and call
	    // tzsetchar():
	    //    char tzInfo[] = "PST8PDT7,M3.2.0/02:00:00,M11.1.0/02:00:00";
	    //    tzsetchar(tzInfo);

	    displayMenu();
	    while (1)
	    {
	        iprintf("\r\nEnter command:  ");
	        char cmdOption = getchar();
	        processCommand(cmdOption);
	    }
}

