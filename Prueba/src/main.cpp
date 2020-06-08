/*NB_REVISION*/

/*NB_COPYRIGHT*/

#include <predef.h>
#include <hal.h>        // For ForceReboot() function
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <nettypes.h>
#include <nbtime.h>
#include <iosys.h>
#include <string.h>
#include <timezones.h>


/*---------------------------------------------------------------------------------------------
 * Display the current system time values, GMT, UTC, Local and seconds since boot.
 *
 * Time types:
 * struct tm    calendar time type (struct)
 * time_t       calendar time since epoch type (typedef)
 * clock_t      processor time since era (typedef)
 *
 *---------------------------------------------------------------------------------------------*/
void displaySystemTime()
{
    iprintf("\r\n\n*** System Time Information ***\r\n");

    time_t timeT;          // Stores the system time in time_t format
    struct tm stmUTC;      // Stores UTC time in struct tm format
    struct tm stmLocal;    // Stores local time as struct tm
    char buffer[128];      // For displaying time strings


    /* time()
     * Returns the current calendar time encoded as a time_t object, and also stores it
     * in the time_t object pointed to by arg (unless arg is a null pointer).
     */
    timeT = time(NULL);   // Get system time as time_t
    iprintf("Seconds obtained from time(): time_t = %lld\r\n", timeT);
    iprintf("Seconds since program start = %ld\r\n", Secs);

/************************************UTC TIME*********************************************/

    /* gmtime_r()
     * Converts given time since epoch (a time_t value pointed to by timeT) into calendar time,
     * expressed in Coordinated Universal Time (UTC) in the struct tm format. The result is
     * stored in the struct tm pointed to by stmUTC.
     */
    gmtime_r(&timeT, &stmUTC);
    iprintf("Time obtainted from gmtime_r(): \r\n");
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S", &stmUTC);
    iprintf("     UTC Time   = %s\r\n", buffer);


    /********************************************CST TIME*********************************************/
    /* localtime_r()
     * Converts given time since epoch (a time_t value pointed to by timeT) into calendar time,
     * expressed in local time, in the struct tm format. The result is stored in the struct tm
     * pointed to by stmLocal.
     */
    localtime_r(&timeT, &stmLocal);
    iprintf("Time obtainted from localtime_r(): \r\n");
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z [%z offset UTC]", &stmLocal);
    iprintf("     Local Time = %s\r\n", buffer);


    /* mktime()
     * Renormalizes local calendar time expressed as a struct tm object and also converts it
     * to time since epoch as a time_t object. time->tm_wday and time->tm_yday are ignored.
     * The values in time are not checked for being out of range.
     *
     * A negative value of time->tm_isdst causes mktime to attempt to determine if Daylight
     * Saving Time was in effect in the specified time.
     *
     * If the conversion to time_t is successful, the time object is modified. All fields of
     * time are updated to fit their proper ranges. time->tm_wday and time->tm_yday are
     * recalculated using information available in other fields.
     */
    // Some time functions modify the struct tm passed to the function, so update everything.
    iprintf("Time values using mktime():\r\n");
    timeT = time(NULL);
    gmtime_r(&timeT, &stmUTC);
    localtime_r(&timeT, &stmLocal);

    time_t timeTmktimeU = mktime(&stmUTC);
    iprintf("     Seconds obtained from mktime() UTC:    time_t = %lld\r\n", timeTmktimeU);

    time_t timeTmktimeL = mktime(&stmLocal);
    iprintf("     Seconds obtained from mktime() Local : time_t = %lld\r\n", timeTmktimeL);

    double timeDiff = difftime(timeTmktimeU, timeTmktimeL);
    iprintf("     Difference UTC - Local: %0.2f sec (%0.2f hours)\r\n", timeDiff, timeDiff/3600.0);
    iprintf("     Daylight savings: %d\r\n", stmLocal.tm_isdst);

    iprintf("\r\n\n");
}



/*------------------------------------------------------------------------------------------
 * Set system time using NTP
 *-----------------------------------------------------------------------------------------*/
void setSystemTimeNTP()
{
    iprintf("Setting system time using NTP...\r\n");
    if (SetTimeNTPFromPool(true))
    {
        iprintf("Success\r\n");
    }
    else
    {
        iprintf("Failed\r\n");
    }
}


/*------------------------------------------------------------------------------------------
   Set the system time manually

   Prompt the user for the values to enter into the tm structure:

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


 *-----------------------------------------------------------------------------------------*/
void setSystemTimeManual()
{
    struct tm stmNewTime;
    static char buffer[128];

    time_t timeT = time(NULL);
    localtime_r(&timeT, &stmNewTime);
    strftime(buffer, sizeof(buffer), "GMT or Local time (tz = %Z)]? (G/L)4", &stmNewTime);
    iprintf("%s\r\n", buffer);

    char c = toupper(getchar());

    if ( c == 'L')
    {
        strftime(buffer, sizeof(buffer), "Setting time in Local time (%Z)", &stmNewTime);
        iprintf("%s\r\n", buffer);
    }
    else
    {
        iprintf("Setting time in GMT\r\n");
    }

    stmNewTime.tm_isdst = -1;       // 0 = std, 1 = dst, -1 = determine which

    int i;

    iprintf("\r\nYear (yyyy)?");
    scanf("%d", &i);
    stmNewTime.tm_year = i - 1900;

    iprintf("\r\nMonth mm (1 - 12)?");
    scanf("%d", &i);
    stmNewTime.tm_mon = (int) i - 1;

    iprintf("\r\nDay of Month dd (1 - 31 etc)?");
    scanf("%d", &i);
    stmNewTime.tm_mday = (int) i;

    iprintf("\r\nHour hh (0 - 23)?");
    scanf("%d", &i);
    stmNewTime.tm_hour = i;

    iprintf("\r\nMinute mm (0 - 59)?");
    scanf("%d", &i);
    stmNewTime.tm_min = i;

    iprintf("\r\nSeconds ss (0 - 59)?");
    scanf("%d", &i);
    stmNewTime.tm_sec = i;

    // scanf() does not process the \n char from user input, so remove it
    // from the buffer here.
    char ch;
    scanf("%c", &ch);

    time_t t = 0;
    if ( c == 'L')
    {
        iprintf("Local time set\r\n");
        t = mktime(&stmNewTime);
    }
    else
    {
        iprintf("GMT time set\r\n");
        t = timegm(&stmNewTime);
    }

    set_time(t);
    iprintf("\r\n\r\n");
}


/*------------------------------------------------------------------------------------------
 * Set system time zone
 *-----------------------------------------------------------------------------------------*/
void setSystemTimeZone()
{
    int i = 0;
    int v = 0;

    iprintf("Select US or International time zones? (U/I)");
    char c = toupper(getchar());

    if (c == 'I')   // International
    {
        while (TZRecords[i].Posix)
        {
            iprintf("%d: %s, %s\r\n", i, TZRecords[i].Name, TZRecords[i].Description);
            i++;
        }

        scanf("%d", &v);
        if (v < i)
            tzsetchar((char *) TZRecords[v].Posix);

        iprintf("Set Time to[%s]\r\n", TZRecords[i].Name);

    }
    else        // US
    {
        int nrecs = 0;
        while (TZRecords[i].Posix)
        {
            if (TZRecords[i].bUsCanada)
                iprintf("%d: %s , %s\r\n", nrecs++, TZRecords[i].Name, TZRecords[i].Description);
            i++;
        }

        scanf("%d", &v);
        if (v < nrecs)
        {
            nrecs = 0;
            i = 0;
            while (TZRecords[i].Posix)
            {
                if (TZRecords[i].bUsCanada)
                {
                    if (nrecs == v)
                    {
                        tzsetchar((char *) TZRecords[i].Posix);
                        iprintf("Set Time to[%s]\r\n", TZRecords[i].Name);
                        return;
                    }
                    else
                    {
                        nrecs++;
                    }
                }
                i++;
            }
        }
    }
}


/*------------------------------------------------------------------------------
 * Display the serial port menu
 *------------------------------------------------------------------------------*/
void displayMenu()
{
    iprintf("1. Display system time\r\n");
    iprintf("2. Set system time using NTP\r\n");
    iprintf("3. Set local time zone\r\n");
    iprintf("4. Set system time manually\r\n");
    iprintf("5. Test time zone\r\n");
    iprintf("0. Reboot system\r\n");
}


// Reference to time zone test function
void testTimeZone();

/*------------------------------------------------------------------------------
 * Serial menu command processor
 *------------------------------------------------------------------------------*/
void processCommand(char command)
{
    switch (command)
    {
        case '1': displaySystemTime();   break;
        case '2': setSystemTimeNTP();    break;
        case '3': setSystemTimeZone();   break;
        case '4': setSystemTimeManual(); break;
        case '5': testTimeZone();        break;
        case '0': ForceReboot();         break;
        default : displayMenu();
    }
}



/*------------------------------------------------------------------------------
 * UserMain
 *------------------------------------------------------------------------------*/
void UserMain(void *pd)
{
    init();

    iprintf("Application started\n");

    displayMenu();

    while (1)
    {
        if (charavail())
        {
            char c = getchar();
            processCommand(c);
            iprintf("\r\n\r\n");
        }
    }
}


