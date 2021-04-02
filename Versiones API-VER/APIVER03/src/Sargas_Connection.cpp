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

/*****************************************SARGAS TCP INFORMATION VARS******************************/
int fd; //file descriptor which controls the tcp
IPADDR ipAddress;//current sargas ip

#define RX_BUFSIZE (4096)//buff size for data received through tcp
char RXBuffer[RX_BUFSIZE];//buff for data received through tcp
//NBString tags;//tags received by tcp


/*****************************************GETS THE CONFIGURATION FROM FLASH VARS******************************/
void getConfig() {
	ipAddress = AsciiToIp(getSargasIp());//get the ipaddress from flash var
}


/*****************************************CLOSES THE TCP CONNECTION******************************/
void closeFd(){
	if (fd > 0)
		closeTcp(fd);//close the fd connection
}


/*****************************************TASK WHICH ATTEND THE TCP READS******************************/
void SargasReads(void * pd) {
	while (1) {
		getConfig();//get the configuratios

		iprintf("Connecting to: %I:%d\r\n", ipAddress, USERPORT);

		closeFd();

		fd = connectTcp(ipAddress, USERPORT);//connects the tcp return >0 for success -1 for fail
		if (fd < 0) {
			iprintf("Error: Connection failed, return code: %d\r\n", fd);
		} else {
			int n = 0;
			bool msgsend=false;
			do {
				n = ReadWithTimeout(fd, RXBuffer, RX_BUFSIZE, 5000);//return 0 for timeout, -1 invalid fd and >0 for bytes lenght read
				RXBuffer[n] = '\0';

				iprintf("Read %d bytes: %s\n", n, RXBuffer);

				if(!msgsend){
					msgsend = true;
				}else{
					if(n>0 ){
						Report((NBString)RXBuffer);
					}
				}
			} while (n > 0);//repeat while n > 0
			iprintf("Sargas Connection close\n");

		}
		OSTimeDly(TICKS_PER_SECOND);
	}
}


/*****************************************TURNS ON THE SARGAS'S RF******************************/
const unsigned char offRf[] = { 0x23, 0x72, 0x65, 0x61, 0x64, 0x65, 0x72, 0x2E, 0x72, 0x66, 0x2E, 0x6F, 0x66, 0x66, 0x0D };
void SargasRfOff() {

	int fd2 = connectTcp( ipAddress, ADMINPORT);

	int n = write( fd2, (char *)&offRf, sizeof(offRf));

	iprintf( "Wrote %d bytes\r\n", n );

	closeTcp(fd2);

	fd2 = 0;
}


/*****************************************TURNS OFF THE SARGAS'S RF******************************/
const unsigned char onRf[] = { 0x23, 0x72, 0x65, 0x61, 0x64, 0x65, 0x72, 0x2E, 0x72, 0x66, 0x2E, 0x6F, 0x6E, 0x0D };
void SargasRfOn() {

	int fd2 = connectTcp( ipAddress, ADMINPORT);

	int n = write( fd2, (char *)&onRf, sizeof(onRf));

	iprintf( "Wrote %d bytes\r\n", n );

	closeTcp(fd2);

	fd2 = 0;
}


/*****************************************GETS THE HEX VALUE FROM A CHAR******************************/
unsigned char getHex(char h){
	uint8_t i;
	char h2[4];
	sprintf(h2, "0x%02X", h);
	i = (uint8_t) strtol(h2, NULL, 0);

	int d;
	unsigned char c;
	sscanf(h2,"0x%02x", &d);
	c=(unsigned char)d;
	return c;
}


/******************PROCESSES AND SENDS THE TIME AND DATE NETBURNER´S INFORMATION TO THE SARGAS'S INFORMATION THROUGH TCP CONNECTION USING THE 2711 PORT********************/
void processTimeAndDate(struct tm &stmLocal){
	//ch = char hour, cm = char minute, cs char second
	char ch[2];
	if(stmLocal.tm_hour<10){
		sprintf(ch, "0%d", stmLocal.tm_hour);
	}else{
		sprintf(ch, "%d", stmLocal.tm_hour);
	}

	char cm[2];
	if(stmLocal.tm_min<10){
		sprintf(cm, "0%d", stmLocal.tm_min);
	}
	else{
		sprintf(cm, "%d", stmLocal.tm_min);
	}

	char cs[2];
	if(stmLocal.tm_sec<10){
		sprintf(cs, "0%d", stmLocal.tm_sec);
	}else{
		sprintf(cs, "%d", stmLocal.tm_sec);
	}

	const unsigned char hour[] = { 0x23, 0x32, 0x30,
			getHex(ch[0]), getHex(ch[1]), 0x3a, getHex(cm[0]), getHex(cm[1]), 0x3a, getHex(cs[0]), getHex(cs[1]),
			 0x0D };

	//cd = char day, cmo = char month, cy char year
	char cd[2];
	if(stmLocal.tm_mday<10){
		sprintf(cd, "0%d", stmLocal.tm_mday);
	}else{
		sprintf(cd, "%d", stmLocal.tm_mday);
	}

	char cmo[2];
	if(stmLocal.tm_mon<10){
		sprintf(cmo, "0%d", stmLocal.tm_mon+1);
	}
	else{
		sprintf(cmo, "%d", stmLocal.tm_mon+1);
	}

	char cy[4];
	sprintf(cy, "%d", stmLocal.tm_year);

	const unsigned char date[] = { 0x23, 0x32, 0x31,
			getHex(cmo[0]), getHex(cmo[1]), 0x2F, getHex(cd[0]), getHex(cd[1]), 0x2F, getHex(cy[1]), getHex(cy[2]),
			0x0D };

	int fd2 = connectTcp( ipAddress, ADMINPORT);

	int n = write( fd2, (const char *)&hour, sizeof(hour));

	n += write( fd2, (const char *)&date, sizeof(date));

	iprintf( "Wrote %d bytes\r\n", n );

	close(fd2);

	fd2 = 0;
}


/*****************************************GETS THE SYSTEM´S TIME AND SENDS IT FOR PROCESS AND AFTER SENDS TO SARGAS******************************/
void SargasSetTimeAndDate() {

	time_t timeT;          // Stores the system time in time_t format
	struct tm stmLocal;    // Stores local time as struct tm
	timeT = time(NULL);   // Get system time as time_t
	localtime_r(&timeT, &stmLocal);

	processTimeAndDate(stmLocal);
}


/*****************************************START THE TCP READ TASK******************************/
void initSargasConnection() {
	//Sargas_Connection::sargas_connection = *this;
	getConfig();

	OSTaskCreatewName(SargasReads, (void *) NULL,
			&SargasTcpTaskStack[USER_TASK_STK_SIZE], SargasTcpTaskStack,
			(MAIN_PRIO + 1), "TCP");
}

