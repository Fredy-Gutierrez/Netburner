/*
 * IO_Interface.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include <stdio.h>
#include <pins.h>
#include <smarttrap.h>
#include <nbrtos.h>
#include <pin_irq.h>
#include <libs/IO_Interface.h>

PinIO inputPins[] = { P2[6], P2[7], P2[8], P2[9], P2[10], P2[11], P2[12],
		P2[13], P2[15], P2[16], P2[17], P2[18], P2[19], P2[20], P2[21], P2[22],
		P1[31], P1[33] };

PinIO outputPins[] = { P1[6], P1[7], P1[8] };

void printInputs() {
	iprintf("\r\n*****Current Inputs*****\r\n");
	for (int x = 0; x < (int) (sizeof(inputPins) / sizeof(inputPins[0])); x++) {

		BOOL bpinstate = inputPins[x].read();

		iprintf("Input #%d: %d\n", x + 1, bpinstate);
	}
	iprintf("\r\n");
}

void configureIO() {
	//configure inputs
	for (int x = 0; x < (int) (sizeof(inputPins) / sizeof(inputPins[0])); x++) {
		inputPins[x].function(PinIO::PIN_FN_IN);
	}

	//configure outputs
	for (int x = 0; x < (int) (sizeof(outputPins) / sizeof(outputPins[0]));
			x++) {
		outputPins[x].function(PinIO::PIN_FN_OUT);
	}
}

void initIO() {
	configureIO();
}

