/*
 * I2C_Controller.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_I2C_CONTROLLER_H_
#define LIBS_I2C_CONTROLLER_H_

//<#includes>
#include <basictypes.h>
//</#includes>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	void initI2C_Controller();

	BOOL readI2C(uint8_t I2C_ADDRESS, uint8_t registerAdd, uint8_t *buf, uint32_t nbytes);

	BOOL writeI2C(uint8_t I2C_ADDRESS, uint8_t registerAdd, uint8_t *buf, uint32_t nbytes);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* LIBS_I2C_CONTROLLER_H_ */
