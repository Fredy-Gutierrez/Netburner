#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <init.h>
#include <iosys.h>
#include <serial.h>
#include <pins.h>
#include <quadspi.h>


// Name for development tools to identify this application
const char * AppName="Serial2SPI using QuadSPI in SPI mode";

//Initialize buffers
static uint8_t RXBuffer[10000], TXBuffer[10000];

// Main task
void UserMain( void * pd)
{
    init();

    // Configure QuadSPI pins for SPI mode
    P2[48].function(PINP2_48_QCS);      // Chip Select
    P2[45].function(PINP2_45_QSCK);     // CLOCK
    P2[47].function(PINP2_47_QIO1);     // QSPI MISO
    P2[43].function(PINP2_43_QIO0);     // QSPI MOSI

    // The QuadSPI SPI mode functionality can be tested with a simple jumper
    // from P2[47] to P2[43] on the MOD-DEV70/100

    // Create and initialize semaphore for SPI (optional)
    OS_SEM QUADSPI_SEM;

    /* Initialize QuadSPI peripheral for Single-bit SPI with 2MHz bus speed and default SPI configuration:
     *
     * SPI_QSPI( uint8_t QSPIModule, uint32_t baudRateInBps,
     *         uint8_t transferSizeInBits = 8, uint8_t peripheralChipSelects = 0x00,
     *         uint8_t chipSelectPolarity = 0x0F, uint8_t clockPolarity = 0,
     *         uint8_t clockPhase = 1, BOOL doutHiz = TRUE,
     *         uint8_t csToClockDelay = 0, uint8_t delayAfterTransfer = 0 );
     */
    SPI_QSPI mySPIObject(DEFAULT_QUADSPI_MODULE, 2000000);

    // Register the semaphore with the SPI driver to be able to pend on transaction completion instead of polling.
    mySPIObject.RegisterSem(&QUADSPI_SEM);

    // Open UART1 and get FD
    int uartFd = OpenSerial( 1, 115200, 1, 8, eParityNone );

    iprintf( "%s application started\r\n", AppName );

    while ( 1 )
    {
        if(dataavail(uartFd))
        {
            int num = read(uartFd, (char*)TXBuffer, 10000); // Read data from UART1

            mySPIObject.Start(TXBuffer, RXBuffer, num);     // Write data read from UART1 to the QuadSPI module

            QUADSPI_SEM.Pend( 0 );                          // Wait for SPI transaction to complete

            writeall(uartFd, (char*)RXBuffer, num);         // Send SPI RX via UART1
        }
    }
}
