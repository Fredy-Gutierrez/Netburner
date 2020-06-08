/*NB_REVISION*/

/*NB_COPYRIGHT*/

#include <predef.h>
#include <stdio.h>
#include <init.h>
#include <pins.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <i2c.h>
#include <time.h>
#include <nettypes.h>
#include <nbtime.h>

const char *AppName = "MODM7AE70 External RTC Using I2C Class";

                                    // Rev 1.93 of the DEV70 has an RTC with an address of 0x51
#define RTC_I2C_ADDR      (0x51)    // I2C address of NXPPCF8563 RTC
#define I2C_BUS_SPEED     (100000)  // 100kHz bus speed

uint8_t RTCdata[30];

const char strTimeZones[][128] = {
        "EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00",
        "CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00",
        "MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00",
        "PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00",
        ""
};

uint8_t MakeBCD(uint32_t i)
{
    uint8_t bv = i % 10;
    bv |= ((i / 10) % 10) * 16;
    return bv;
}

uint32_t GetBCD(uint8_t bv)
{
    uint32_t i = bv & 0x0F;
    i += 10 * ((bv >> 4) & 0x0f);
    return i;
}

uint32_t GetTime_NXPPCF8563(struct tm &bts)
{
    RTCdata[0] = 0x2;

    if(i2c[0].readRegN(RTC_I2C_ADDR, RTCdata[0], RTCdata, 7) != I2C::I2C_RES_ACK)
    {
        iprintf("Error reading from RTC\r\n");
        return 1;
    }

    bts.tm_sec = GetBCD(RTCdata[0] & 0x7F);
    bts.tm_min = GetBCD(RTCdata[1] & 0x7F);
    bts.tm_hour = GetBCD(RTCdata[2] & 0x3F);
    bts.tm_mday = GetBCD(RTCdata[3] & 0x3F);
    bts.tm_wday = GetBCD(RTCdata[4] & 0x07);
    bts.tm_mon = (GetBCD(RTCdata[5] & 0x1F) - 1);

    if (RTCdata[5] & 0x80) { bts.tm_year = GetBCD(RTCdata[6]); }
    else
    {
        bts.tm_year = (GetBCD(RTCdata[6]) + 100);
    }

    return 0;
}

uint32_t SetTime_NXPPCF8563(struct tm &bts)
{
    RTCdata[0] = 0x2;
    RTCdata[1] = MakeBCD(bts.tm_sec);
    RTCdata[2] = MakeBCD(bts.tm_min);
    RTCdata[3] = MakeBCD(bts.tm_hour);
    RTCdata[4] = MakeBCD(bts.tm_mday);
    RTCdata[5] = bts.tm_wday;
    RTCdata[6] = MakeBCD(bts.tm_mon + 1);

    if (bts.tm_year > 99)
        RTCdata[7] = MakeBCD(bts.tm_year - 100);
    else
    {
        RTCdata[6] |= 0x80;
        RTCdata[7] = MakeBCD(bts.tm_year);
    }

    if(i2c[0].writeRegN( RTC_I2C_ADDR, RTCdata[0], RTCdata+1, 7) != I2C::I2C_RES_ACK)
    {
        return 1;
    }

    return 0;
}

int RTCSetRTCfromSystemTime(void)
{
    OSLockObj lock;
    time_t t = time(nullptr);
    struct tm *ptm = gmtime(&t);
    return SetTime_NXPPCF8563(*ptm);
}

int RTCSetSystemFromRTCTime( void )
{
   struct tm bts;
   int rv = GetTime_NXPPCF8563( bts );

   if ( rv )
   {
      return rv;
   }

   time_t t = timegm(&bts);
   set_time( t );
   return 0;
}

void SetTimeRTC()
{
   OSLockObj lock;
   /* Synchronize the RTC with the system time. */
   int n = RTCSetSystemFromRTCTime();
   if ( n == 0 )
   {
      iprintf( "Set system time from RTC\r\n" );
   }
   else
   {
      iprintf( "Error: could not set system time from RTC\r\n" );
   }
}

void DisplaySystemTime()
{
    time_t RawTime;
    struct tm *UTCTimeInfo;
    struct tm *LocalTimeInfo;

    RawTime = time(nullptr);               // Get system time in time_t format
    UTCTimeInfo = gmtime(&RawTime);        // expects tm* param to be in UTC
    LocalTimeInfo = localtime(&RawTime);   // Convert to struct tm format
    iprintf("Current time: %s", asctime(UTCTimeInfo));
    iprintf("Current Local time: %s", asctime(LocalTimeInfo));

    // Example of strftime() usage
    char buf[80];
    /* size_t strftime(char *s,
                       size_t maxsize,
                       const char *format,
                       const struct tm *timp);
    */
    strftime(buf, 79, "%l:%M:%S %p", UTCTimeInfo);
    iprintf("Formatted time string: %s\r\n", buf);
    LocalTimeInfo = localtime(&RawTime);
    strftime(buf, 79, "%l:%M:%S %p", LocalTimeInfo);
    iprintf("Formatted Local time string: %s\r\n", buf);
}

void DisplayRTCTime()
{
    time_t RawTime;
    struct tm UTCTimeInfo;
    struct tm *LocalTimeInfo;

    uint8_t ret = GetTime_NXPPCF8563(UTCTimeInfo);
    if(ret)
    {
        iprintf("Error getting RTC time\r\n");
        return;
    }
    RawTime = timegm(&UTCTimeInfo);
    LocalTimeInfo = localtime(&RawTime);

    iprintf("Current RTC time: %s", asctime(&UTCTimeInfo));
    iprintf("Current Local RTC time: %s", asctime(LocalTimeInfo));

    // Example of strftime() usage
    char buf[80];
    /* size_t strftime(char *s,
                       size_t maxsize,
                       const char *format,
                       const struct tm *timp);
    */
    strftime(buf, 79, "%l:%M:%S %p", &UTCTimeInfo);
    iprintf("Formatted RTC time string: %s\r\n", buf);
    LocalTimeInfo = localtime(&RawTime);
    strftime(buf, 79, "%l:%M:%S %p", LocalTimeInfo);
    iprintf("Formatted Local RTC time string: %s\r\n", buf);
}



void ScanI2CBus()
{
    uint8_t readData = 0;
    uint8_t ret = 0;

    for(uint8_t addr = 0; addr < 128; addr++)
    {
        ret = i2c[0].readReg8(addr, 0x0 /* reg */, readData);
        if(ret == I2C::I2C_RES_ACK)
        {
            iprintf("Found a device at address 0x%02x\r\n", addr);
        }
    }
}

void SetTimeManual(int month, int day, int year, int weekday,int dyear, int hour, int min, int sec)
{

    struct tm SetTime;

    SetTime.tm_mon = month;
    SetTime.tm_mday = day;
    SetTime.tm_wday = weekday;
    SetTime.tm_year = year - 1900;
    SetTime.tm_yday = dyear;

    SetTime.tm_hour = hour;
    SetTime.tm_min = min;
    SetTime.tm_sec = sec;

    /* set_time() need a parameter of time_t, so use mktime() to convert
       a struct tm type to a time_t type.
    */
    set_time(mktime(&SetTime));
}

void DisplayMenu( void )
{
   iprintf( "\r\n----- Main Menu -----\r\n" );
   iprintf( "D - Display system time\r\n" );
   iprintf( "C - Display RTC time\r\n");
   iprintf( "R - Use RTC to set System Time\r\n" );
   iprintf( "S - Set External RTC to current system time\r\n" );
   iprintf( "T - Set time zone (PDT = -7)\r\n" );
   iprintf( "? - Display Menu\r\n" );

}

void ProcessCommand(char cmd)
   {
    switch (toupper(cmd)) {
      case 'D':
    	 SetTimeManual(4,20,2020,21,141,12,27,10);
         DisplaySystemTime();
         break;

     case 'C':
         DisplayRTCTime();
         break;

      case 'R':
         SetTimeRTC();
         break;

      case 'S':
         // If you are using NTP or the Manual method to set time, and want to
         // set the RTC to the current value, you can use the function:
         iprintf("Setting external RTC to current system time...");
         RTCSetRTCfromSystemTime();
         iprintf("complete\r\n");
         break;

    case 'T': {
         char buf[80];

        int i = 0;
        while (strTimeZones[i][0] != '\0')
        {
            iprintf("%d. %s\r\n", i, strTimeZones[i]);
            i++;
        }

        iprintf("Select time zone ");
        char c = getchar();
        int j = c - 0x30;
        iprintf("\r\nSetting time zone to :[%s]\r\n", (char *)strTimeZones[j]);
        tzsetchar((char*) (char *)strTimeZones[j]);
      }
         break;

      case 'W':
         iprintf( "Tick Count = 0x%lX = %ld (%ld seconds)\r\n", TimeTick, TimeTick, ( TimeTick / TICKS_PER_SECOND ) );
         break;

     default: // '?'
      DisplayMenu();
   }
}

void UserMain(void *pd)
{
    init();
    EnableSmartTraps();

    iprintf("Application Started\r\n");
    OSTimeDly(TICKS_PER_SECOND);

    P2[39].function(PINP2_39_TWD0);
    P2[42].function(PINP2_42_TWCK0);

    i2c[0].setup(I2C_BUS_SPEED);

    DisplayMenu();
    while ( 1 )
    {
       char c = getchar();
       ProcessCommand( c );
    }
}