
#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <startnet.h>
#include <dhcpclient.h>
#include <taskmon.h>
#include <smarttrap.h>
#include <serial.h>
#include <pins.h>
#include <dspi.h>

#define BUFFER_SIZE 10000

// Instruct the C++ compiler not to mangle the function name
extern "C"
{
void UserMain( void * pd);
}

// Name for development tools to identify this application
const char * AppName="DSPIMux";

// Main task
void UserMain( void * pd)
{
    init();
    EnableTaskMonitor();
    EnableSmartTraps();

#ifdef __DEBUG
    InitializeNetworkGDB();
#endif

    iprintf( "%s application started\r\n", AppName );

    //Initialize buffers
    static uint8_t RXBuffer[BUFFER_SIZE], TXBuffer[BUFFER_SIZE],RXBuffer2[BUFFER_SIZE];

     P2[27].setFn(PINP2_27_SPI0_MISO);     // SPI0_MISO
     P2[28].setFn(PINP2_28_SPI0_MOSI);     // SPI0_MOSI
     P2[25].setFn(PINP2_25_SPI0_SPCK);     // SPI0_SPCK
     P2[29].setFn(PINP2_29_SPI0_NPCS0);    // SPI0_NPCS0
     P2[30].setFn(PINP2_30_SPI0_NPCS2);    // SPI0_NPCS2
    //    P2[40].function(PINP2_40_SPI0_NPCS1); // SPI0_NPCS1
    //    P2[26].function(PINP2_26_SPI0_NPCS3); // SPI0_NPCS3

    // The DSPI functionality can be tested with a simple jumper
    // from J2[27] to J2[28] on the MOD-DEV70/100 or from Pins[33] to Pins[35]
    // on the NANO54415 carrier board.

    // Create and initialize semaphore for DSPI (optional)
    OS_SEM AudioSem, SPIFlashSem;
    OSSemInit(& AudioSem, 0);
    OSSemInit(& SPIFlashSem, 0);

    // SPIModule( uint8_t SPIModule, uint32_t baudRateInBps,
    //         uint8_t transferSizeInBits = 8, uint8_t peripheralChipSelects = 0x00,
    //         uint8_t chipSelectPolarity = 0x0F, uint8_t clockPolarity = 0,
    //         uint8_t clockPhase = 1, BOOL doutHiz = TRUE,
    //         uint8_t csToClockDelay = 0, uint8_t delayAfterTransfer = 0 );

    // Set the baudrate to 500KHz, 8bit mode, inactive on all CS is High, assert CS0 Low
    SPIModule AudioSPI(0, 500000, 8, 0);
    AudioSPI.RegisterSem( &AudioSem );

    // Set the baudrate to 10 MHz, 8bit mode, inactive on all CS is High, assert CS2 Low
    SPIModule SPIFlashIC(0, 10000000, 8, 2);
    SPIFlashIC.RegisterSem( &SPIFlashSem );

    for (int i = 0; i < BUFFER_SIZE; i++) {
        TXBuffer[i] = (uint8_t)i;
    }

    bool SpiTarget = true;
    while ( 1 )
    {
        if (SpiTarget) {
            iprintf("Reading \"AudioSPI\"\r\n");
            uint8_t audioSPIRet = AudioSPI.Start(TXBuffer, RXBuffer, 10000);
            AudioSem.Pend(0);
            iprintf("AudioSem posted %i\r\n",RXBuffer[0]);
        }
        else {
            iprintf("Writing to \"SPIFlashIC\"\r\n");
            uint8_t flashSPIRet = SPIFlashIC.Start((uint8_t*)TXBuffer, RXBuffer2, 10000);
            SPIFlashSem.Pend(0);
            iprintf("SPIFlashSem posted %i\r\n",RXBuffer2[0]);
        }
        SpiTarget = !SpiTarget;
        OSTimeDly( 2 * TICKS_PER_SECOND );
    }
}
