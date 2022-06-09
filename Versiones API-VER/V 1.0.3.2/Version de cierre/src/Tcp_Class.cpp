/*
 * TcpClass.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#include <nettypes.h>
#include <constants.h>
#include <tcp.h>
#include <iosys.h>
#include <string.h>
#include "libs/Tcp_Class.h"
#include "libs/Rtc_Controller.h"
#include "libs/Log.h"
#include "libs/Business.h"



/*********************************************CLOSE THE FILE DESCRIPTOR(IN THIS CASE THE TCP'S FD)***********************************************/
BOOL closeTcp(int fd) {

	if (close(fd) != 0) {
		return false;
	}
	return true;
}


/*********************************************CONNECT THE FD TO A TCP CONNECTION***********************************************/
int connectTcp(IPADDR ip, int port) {
	int fd = connect(ip, port, TICKS_PER_SECOND * 5);
	return fd;
}


/*********************************************WRITE IN TCP CONNECTION***********************************************/
BOOL writeTcp(IPADDR ipaddress, int port, char* msg, int bytes) {
	int fd2 = connectTcp(ipaddress, port);

	int n = write(fd2, msg, bytes);

	logg.newPrint("\nWrote %d bytes", n);

	close(fd2);

	fd2 = 0;

	return true;
}


/*********************************************STARTS THE TCP***********************************************/
void initTcp() {
}

