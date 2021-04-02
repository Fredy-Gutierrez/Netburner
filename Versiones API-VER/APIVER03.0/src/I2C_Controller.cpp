/*
 * I2C_Controller.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#include "libs/I2C_Controller.h"
#include "libs/Rtc_Controller.h"
#include <basictypes.h>
#include <i2c.h>
#include <pins.h>

#define I2C_MODULE_NUM    0           // use I2C0/TW0
#define I2C_BUS_SPEED    (100000)     // Bus speed of 100kHz

uint8_t bufData[30];

int8_t initI2CBus(uint8_t i2cModuleNum) {
	int8_t status = 0;

	if ((i2cModuleNum < 0) || (i2cModuleNum > 2)) {
		status = 1;
	} else {

		switch (i2cModuleNum) {
		case 0:
			P2[39].function(PINP2_39_TWD0);
			P2[42].function(PINP2_42_TWCK0);
			i2c[0].setup(I2C_BUS_SPEED);
			break;
		case 1:
			P2[22].function(PINP2_22_TWD1);
			//P2[12].function(PINP2_12_TWCK1);
			i2c[1].setup(I2C_BUS_SPEED);
			break;
		case 2:
			P2[26].function(PINP2_26_TWD2);
			P2[23].function(PINP2_23_TWCK2);
			i2c[2].setup(I2C_BUS_SPEED);
			break;
		default:
			status = 1;
			//iprintf("invalid moduleNum\r\n");
			break;
		}
	}

	return status;
}

BOOL readI2C(uint8_t I2C_ADDRESS, uint8_t registerAdd, uint8_t *buf,
		uint32_t nbytes) {

	I2CDevice i2cDevice(i2c[I2C_MODULE_NUM], I2C_ADDRESS);

	if (i2cDevice.readRegN(registerAdd, buf, nbytes) == I2C::I2C_RES_ACK) {
		return true;
	} else {
		return false;
	}

}

BOOL writeI2C(uint8_t I2C_ADDRESS, uint8_t registerAdd, uint8_t *buf,
		uint32_t nbytes) {

	I2CDevice i2cDevice(i2c[I2C_MODULE_NUM], I2C_ADDRESS);

	if (i2cDevice.writeRegN(registerAdd, buf, nbytes) == I2C::I2C_RES_ACK) {
		return true;   // success
	} else {
		return false;   // failure
	}
}

/***************************************************************INIT******************************************************************/
void initI2C_Controller() {
	initI2CBus(I2C_MODULE_NUM);
	initRtc();
}

