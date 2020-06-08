/*NB_REVISION*/

/*NB_COPYRIGHT*/

/*---------------------------------------------------------------------------------------------
 * Functions to test and display the current time zone settings.
 * NOTE: THE SYSTEM TIME AND TIME ZONE MUST BE SET BEFORE THESE FUNCTIONS ARE CALLED.
 *
 * Before calling these functions:
 * - The system time must be set, using NTP, manually, or a real-time clock.
 * - The system time zone must be set with tzsetchar()
 *
 * The testTimeZone function will use the system time settings to display:
 * - Start of year in GMT
 * - End of year in GMT
 * - Start of year time zone offset
 * - Daylight savings time dates and offsets
 *
 * For example:
 * Start of Year (GMT)= Wed, 01 Jan 2020 00:00:00
 * End of Year (GMT)  = Fri, 01 Jan 2021 00:00:00
 * Offset at at start of year: -28800
 *
 * Daylight Savings Time Changes:
 *   At Time:= Sun, 08 Mar 2020 01:59:59 (-0800)
 *   Offset change is: -28800 to -25200
 *   At Time:= Sun, 01 Nov 2020 01:59:59 (-0700)
 *   Offset change is: -25200 to -28800
 *
 *-------------------------------------------------------------------------------------------------*/
#include <predef.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <nbrtos.h>
#include <nettypes.h>
#include <nbtime.h>
#include <iosys.h>
#include <string.h>
#include <timezones.h>



/*----------------------------------------------------------------------------------------
 * Displays the GMT time of the specified time_t value.
 *----------------------------------------------------------------------------------------*/
void showHumanTimeGMT(const char * label, time_t t)
{
    struct tm tm_tst;
    gmtime_r(&t, &tm_tst);

    char buffer[80];       // For displaying time strings
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S ", &tm_tst);
    iprintf("%s %s\r\n", label, buffer);
}


/*----------------------------------------------------------------------------------------
 * Displays the Local time of the specified time_t value.
 *----------------------------------------------------------------------------------------*/
void showHumanTimeLocal(const char * label, time_t t)
{
    struct tm tm_tst;
    localtime_r(&t, &tm_tst);

    char buffer[80];       // For displaying time strings
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S (%z)", &tm_tst);
    iprintf("%s %s\r\n", label, buffer);
}


/*----------------------------------------------------------------------------------------
 * Returns the time zone offset offset between the current local time and the specified time_t.
 *----------------------------------------------------------------------------------------*/
time_t getOffsetAt(time_t tt)
{
    struct tm tm_local;
    localtime_r(&tt, &tm_local);

    time_t tlcl = timegm(&tm_local);

    return (tlcl - tt);
}


/*----------------------------------------------------------------------------------------
 * Displays the time zone information of the current system time zone settings:
 * - Start of year in GMT
 * - End of year in GMT
 * - Start of year time zone offset
 * - Daylight savings time dates and offsets
 *----------------------------------------------------------------------------------------*/
void testTimeZone()
{
    iprintf("\r\n*********************************************************\r\n");
    iprintf("  Use of this function requires that: \r\n");
    iprintf("  1. The system time is set correctly\r\n");
    iprintf("  2. The time zone is set correctly with tzsetchar()\r\n");
    iprintf("*********************************************************\r\n");

    time_t tt;
    struct tm tmStartOfYear;            // Stores UTC time as struct tm

    tt = time(NULL);                    // Get system time as time_t
    gmtime_r(&tt, &tmStartOfYear);      // Get system time in struct tm format
    tmStartOfYear.tm_sec  = 0;
    tmStartOfYear.tm_min  = 0;
    tmStartOfYear.tm_hour = 0;
    tmStartOfYear.tm_mday = 1;
    tmStartOfYear.tm_mon  = 0;

    time_t ttStartOfYear = timegm(&tmStartOfYear);
    tmStartOfYear.tm_year += 1;
    time_t ttEndOfYear = timegm(&tmStartOfYear);

    iprintf("\r\n");

    char buffer[80];
    struct tm tmLocal;
    localtime_r(&tt, &tmLocal);
    strftime(buffer, sizeof(buffer), "Current Time Zone: %Z (%z)", &tmLocal);
    iprintf("%s\r\n", buffer);

    showHumanTimeGMT("Start of Year (GMT)", ttStartOfYear);
    showHumanTimeGMT("End of Year (GMT)  ", ttEndOfYear);


    time_t offset = getOffsetAt(ttStartOfYear);
    time_t lastOffset = offset;
    iprintf("Offset at at start of year: %lld\r\n", offset);
    iprintf("\r\n");

    // Calculate daylight savings time changes
    iprintf("Daylight Savings Time Changes:\r\n");
    for (time_t ttNow = ttStartOfYear; ttNow < ttEndOfYear; ttNow += 24 * 3600)
    {
        time_t ttNewOffset = getOffsetAt(ttNow);
        if (ttNewOffset != lastOffset)
        {
            // The switch over happened some time in last 24 hrs
            time_t ttPreChange = ttNow - (12 * 3600);
            time_t ttDelta = (6 * 3600);
            while (ttDelta)
            {
                if (getOffsetAt(ttPreChange) == lastOffset)
                {
                    ttPreChange += ttDelta;
                    ttDelta /= 2;
                }
                else
                {
                    ttPreChange -= ttDelta;
                    ttDelta /= 2;
                }
            }

            ttPreChange -= 1;
            while (getOffsetAt(ttPreChange) == lastOffset)
                ttPreChange++;

            showHumanTimeLocal("  At Time:", ttPreChange - 1);
            iprintf("  Offset change is: %lld to %lld\r\n", lastOffset, ttNewOffset);
            lastOffset = ttNewOffset;
        }
    }
}
