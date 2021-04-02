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
extern "C"
{
#endif /* __cplusplus */
//<body>

	/*********************************************GET FROM THE FLASH MEMORY***********************************************/
	const char* getSargasIp();

	const char* getServerUrl();

	int getUdpPort();

	const char* getUdpIp();

	int getTagCounter();


	/********************************************SET TO FLASH MEMORY***********************************************/
	void setUdpIp(const char * newUdpIp);

	void setUdpPort(int port);

	void incrementTagCounter();

	void setServerUrl(const char * newUrl);

	void setSargasIp(const char * newIp);

	void restoreDefault();

	/*CASE: PORT CHANGE
	 * void setSargasPort(const char * newPort);
	 *
	 *int getSargasPort();
	*/

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_CONFIGURATIONS_H_ */
