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
#include "libs/Sargas_Connection.h"



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
BOOL writeTcp(IPADDR ipaddress, int port, char *msg) {

	int fd2 = connectTcp(ipaddress, port);

	int n = write(fd2, msg, strlen(msg));

	iprintf("Wrote %d bytes\r\n", n);

	close(fd2);

	fd2 = 0;

	return true;
}


/*********************************************STARTS THE TCP***********************************************/
void initTcp() {
	initSargasConnection();
}

