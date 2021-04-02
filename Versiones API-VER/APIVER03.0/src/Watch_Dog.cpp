/*
 * Watch_Dog.cpp
 *
 *  Created on: 4 mar 2021
 *      Author: Integra Fredy
 */
#include "libs/Watch_Dog.h"
#include <config_obj.h>
#include <sim.h>
#include <config_server.h>
#include <hal.h>
#include <same70_wdt.h>

extern config_bool watchdog_enabled;

inline void serviceWatchdog() {
	/* Set the Watchdog Restart bit*/
	WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
}

void enableWatchdog(uint16_t counterValue) {
	uint32_t wdt_mr = 0;

	if (counterValue > 0xFFF) {
		counterValue = 0xFFF;
	}
	if (counterValue < 0x001) {
		counterValue = 0x001;
	}

	/* Assign the watchdog_service_function function pointer*/
	watchdog_service_function = &serviceWatchdog;

	wdt_mr = WDT_MR_WDD(0xFFF) | WDT_MR_WDRSTEN | counterValue;

	WDT->WDT_MR = wdt_mr;
}

void initWatchDog() {
	if (watchdog_enabled == false) {
		iprintf(
				"Reconfiguring MODM7AE70 watchdog to be enabled on boot and rebooting the device\r\n");
		watchdog_enabled = true;
		SaveConfigToStorage(); // Save the change to watchdog_enabled config variable to flash

		OSTimeDly(TICKS_PER_SECOND);
		ForceReboot(); // The NBRTOS will not write to the Watchdog Timer Mode Register on next boot
	} else {
		iprintf(
				"The MODM7AE70 watchdog is configured to be enabled on boot\r\n");
	}

	enableWatchdog(0xFFF);
}

