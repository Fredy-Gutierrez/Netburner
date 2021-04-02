/*
 * Configurations.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_CONFIGURATIONS_H_
#define LIBS_CONFIGURATIONS_H_
//<#includes>

#include <basictypes.h>

//</#includes>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
//<body>

const char* getSargasIp();

void setSargasIp(const char * newIp);

void restoreDefault();

/*En caso de cambio de puerto
 * void setSargasPort(const char * newPort);
 *
 *int getSargasPort();
 */

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBS_CONFIGURATIONS_H_ */
