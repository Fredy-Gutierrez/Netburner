/*
 * Tcp_Class.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_TCP_CLASS_H_
#define LIBS_TCP_CLASS_H_

#include <nettypes.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	void initTcp();

	int connectTcp(IPADDR ip, int port);

	BOOL writeTcp(IPADDR ipaddress, int port, char *msg);

	BOOL closeTcp(int fd);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */




#endif /* LIBS_TCP_CLASS_H_ */
