/*
 * SargasConnection.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#include "libs/Sargas_Connection.h"
#include <stdio.h>
#include <tcp.h>
#include <iosys.h>
#include <utils.h>
#include <time.h>
#include <nbstring.h>
#include <string.h>
#include <stdlib.h>
#include <constants.h>
#include "libs/Configurations.h"
#include "libs/Tcp_Class.h"
#include "libs/Default_Values.h"
#include "libs/Report2Server.h"

uint32_t SargasTcpTaskStack[USER_TASK_STK_SIZE];

int fd; //file descriptor which controls the tcp

IPADDR ipAddress;

#define RX_BUFSIZE (4096)
char RXBuffer[RX_BUFSIZE];
NBString tags;

void getConfig() {
	ipAddress = AsciiToIp(getSargasIp());
}

void closeFd() {
	if (fd > 0)
		closeTcp(fd);
}

void SargasReads(void * pd) {
	while (1) {
		getConfig();

		iprintf("Connecting to: %I:%d\r\n", ipAddress, USERPORT);

		if (fd > 0)
			closeTcp(fd);

		fd = connectTcp(ipAddress, USERPORT);

		if (fd < 0) {
			iprintf("Error: Connection failed, return code: %d\r\n", fd);
		} else {
			int n = 0, msgnum = 0;
			do {
				n = ReadWithTimeout(fd, RXBuffer, RX_BUFSIZE, 5000);
				RXBuffer[n] = '\0';

				iprintf("Read %d bytes: %s\n", n, RXBuffer);

				if (msgnum == 0) {
					msgnum = 1;
				} else {
					if (n > 0) {
						Report((NBString) RXBuffer);
						iprintf("reported");
					}
				}

			} while (n > 0);
			iprintf("Sargas Connection close\n");
		}

	}
}

const unsigned char offRf[] = { 0x23, 0x72, 0x65, 0x61, 0x64, 0x65, 0x72, 0x2E,
		0x72, 0x66, 0x2E, 0x6F, 0x66, 0x66, 0x0D };
void SargasRfOff() {

	int fd2 = connectTcp(ipAddress, ADMINPORT);

	int n = write(fd2, (char *) &offRf, sizeof(offRf));

	iprintf("Wrote %d bytes\r\n", n);

	closeTcp(fd2);

	fd2 = 0;
}

const unsigned char onRf[] = { 0x23, 0x72, 0x65, 0x61, 0x64, 0x65, 0x72, 0x2E,
		0x72, 0x66, 0x2E, 0x6F, 0x6E, 0x0D };
void SargasRfOn() {

	int fd2 = connectTcp(ipAddress, ADMINPORT);

	int n = write(fd2, (char *) &onRf, sizeof(onRf));

	iprintf("Wrote %d bytes\r\n", n);

	closeTcp(fd2);

	fd2 = 0;
}

unsigned char getHex(char h) {
	uint8_t i;
	char h2[4];
	sprintf(h2, "0x%02X", h);
	i = (uint8_t) strtol(h2, NULL, 0);

	int d;
	unsigned char c;
	sscanf(h2, "0x%02x", &d);
	c = (unsigned char) d;
	return c;
}

void processTimeAndDate(struct tm &stmLocal) {
	//ch = char hour, cm = char minute, cs char second
	char ch[2];
	if (stmLocal.tm_hour < 10) {
		sprintf(ch, "0%d", stmLocal.tm_hour);
	} else {
		sprintf(ch, "%d", stmLocal.tm_hour);
	}

	char cm[2];
	if (stmLocal.tm_min < 10) {
		sprintf(cm, "0%d", stmLocal.tm_min);
	} else {
		sprintf(cm, "%d", stmLocal.tm_min);
	}

	char cs[2];
	if (stmLocal.tm_sec < 10) {
		sprintf(cs, "0%d", stmLocal.tm_sec);
	} else {
		sprintf(cs, "%d", stmLocal.tm_sec);
	}

	const unsigned char hour[] = { 0x23, 0x32, 0x30, getHex(ch[0]), getHex(
			ch[1]), 0x3a, getHex(cm[0]), getHex(cm[1]), 0x3a, getHex(cs[0]),
			getHex(cs[1]), 0x0D };

	//cd = char day, cmo = char month, cy char year
	char cd[2];
	if (stmLocal.tm_mday < 10) {
		sprintf(cd, "0%d", stmLocal.tm_mday);
	} else {
		sprintf(cd, "%d", stmLocal.tm_mday);
	}

	char cmo[2];
	if (stmLocal.tm_mon < 10) {
		sprintf(cmo, "0%d", stmLocal.tm_mon + 1);
	} else {
		sprintf(cmo, "%d", stmLocal.tm_mon + 1);
	}

	char cy[4];
	sprintf(cy, "%d", stmLocal.tm_year);

	const unsigned char date[] = { 0x23, 0x32, 0x31, getHex(cmo[0]), getHex(
			cmo[1]), 0x2F, getHex(cd[0]), getHex(cd[1]), 0x2F, getHex(cy[1]),
			getHex(cy[2]), 0x0D };

	int fd2 = connectTcp(ipAddress, ADMINPORT);

	int n = write(fd2, (const char *) &hour, sizeof(hour));

	n += write(fd2, (const char *) &date, sizeof(date));

	iprintf("Wrote %d bytes\r\n", n);

	close(fd2);

	fd2 = 0;
}

void SargasSetTimeAndDate() {

	time_t timeT;          // Stores the system time in time_t format
	struct tm stmLocal;    // Stores local time as struct tm
	timeT = time(NULL);   // Get system time as time_t
	localtime_r(&timeT, &stmLocal);

	processTimeAndDate(stmLocal);
}

void initSargasConnection() {
	//Sargas_Connection::sargas_connection = *this;
	getConfig();

	OSTaskCreatewName(SargasReads, (void *) NULL,
			&SargasTcpTaskStack[USER_TASK_STK_SIZE], SargasTcpTaskStack,
			(MAIN_PRIO + 1), "TCP");
}

