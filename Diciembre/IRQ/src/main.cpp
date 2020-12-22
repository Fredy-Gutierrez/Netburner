#include <predef.h>
#include <stdio.h>
#include <pins.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <pin_irq.h>
#include <http.h>
#include <init.h>

const char * AppName="IRQ";

// IRQ signal polarities. Trigger on either edge, rising edge, falling edge, or level sensitive.
#define IRQ_POLARITY_EITHER     0
#define IRQ_POLARITY_RISING     1
#define IRQ_POLARITY_FALLING   -1
#define IRQ_POLARITY_LEVEL      2

// Parallel I/O Ports (PIO). There are a total of 5 ports. Each port supports a number of port pins.
// Please refer to the NetBurner module data sheet for a list of ports and pins. For example,
// on the MODM7AE70, P2[8] is PA17 (Port A, pin 17).
#define PIO_PORT_A   0
#define PIO_PORT_B   1
#define PIO_PORT_C   2
#define PIO_PORT_D   3
#define PIO_PORT_E   4

// Global variables
uint32_t    gIrqCounter     = 0;   // Number of IRQs
int         gRxIrqPort      = 0;
int         gRxIrqPortPin   = 0;
uint32_t    gIrqCounter2     = 0;

/*-----------------------------------------------------------------------------
 * The interrupt service routine (ISR). It must be in the format of: function(int, int).
 * You do not need to use the parameters, but it must have that signature.
 *
 * Parameters:
 *  port    The PIO port number of the received IRQ, 0 - 4.
 *  portPin The PIO pin number of the received IRQ.
 *
 *----------------------------------------------------------------------------*/

void extISR(int port, int portPin)
{
    gRxIrqPort      = port;     // Record the port and port pin for display later
    gRxIrqPortPin   = portPin;
    gIrqCounter++;              // Count the number of IRQs

    BOOL bpinstate = P2[11];

    iprintf("IRQs: %lu, Port: %d, PortPin: %d val: %d\r\n", gIrqCounter, gRxIrqPort, gRxIrqPortPin,bpinstate);
}

void extISR0(int port, int portPin)
{
    gIrqCounter2++;              // Count the number of IRQs

    iprintf("IRQs2: %lu, Port: %d, PortPin: %d\r\n", gIrqCounter2, port, portPin);
}

void UserMain(void * pd)
{
	const int outputPinNum  = 15;   // Pin P2.15 is connected to the first LED on the MOD-DEV-70 development board
	const int toggleSeconds = 1;
	PinIO pioPort = P2[11];          // IRQ input port. Refer to NetBurner module data sheet

    init();
    EnableSmartTraps();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait up to 5 seconds for active network activity 
    StartHttp();

    //iprintf("Application %s started\n",AppName );
    iprintf("Configuring P2 pin %d as GPIO, will toggle at %d seconds\r\n", outputPinNum, toggleSeconds);
    P2[outputPinNum].function(PinIO::PIN_FN_OUT);     // PinIO::PIN_FN_OUT = GPIO function

        // Configure pin for detecting external interrupts
    SetPinIrq( pioPort, IRQ_POLARITY_EITHER, extISR);

    while (1)
    {
    	P2[outputPinNum] = 1;

    	OSTimeDly(TICKS_PER_SECOND * toggleSeconds);

    	P2[outputPinNum] = 0;

    	OSTimeDly(TICKS_PER_SECOND * toggleSeconds);

    	//iprintf("IRQs: %lu, Port: %d, PortPin: %d\r\n", gIrqCounter, gRxIrqPort, gRxIrqPortPin);
    }
}
