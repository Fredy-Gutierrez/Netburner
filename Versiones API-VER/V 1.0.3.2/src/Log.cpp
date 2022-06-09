/*
 * SpeedProcess.cpp
 *
 *  Created on: 21 april 2021
 *      Author: Integra MGZ
 */


#include <nbstring.h>
#include <syslog.h>
#include <stdio.h>
#include <utils.h>
#include <udp.h>
#include "libs/Log.h"
#include "libs/Configurations.h"
#include "libs/fifo/Fifo.h"
#include <stdlib.h>

Fifo fifoMsg;

logger::logger(){
	stream=strBoth;
	printMsg=FALSE;
	AddingMessage = false;
}

void logger::setStream(int str){
	stream=(streamDest)str;
}
void logger::setMsgPrn(bool prn){
	printMsg=prn;
}
bool logger::getMsgPrn(){
	return printMsg;
}

void logger::prn(char * strToPrn){
	char buffer[1000];

	stream=(streamDest)getLogChoice();
	if(stream==strNone) return;

	if(stream==strSerial || stream==strBoth)
		iprintf("%s", strToPrn);
	if(stream==strUdpLog || stream==strBoth){
		sprintf(buffer, "%s", strToPrn);
		sendUdp(buffer);
	}
}

void logger::newPrint(char * format, ...){
	if(AddingMessage){
		fifoMsg.Add((void*) OVERCALLMESSAGE);
		return;
	}
	AddingMessage = true;
	char buf[1024];
	memset(buf,'\0',1024);

	va_list vl;
	va_start(vl, format);
	vsnprintf( buf, sizeof( buf), format, vl);
	va_end( vl);

	int arrLen = strlen(buf);

	char * buffer = ( char* )malloc( sizeof( char ) * (arrLen + 1) );
	memcpy(buffer, buf, arrLen);
	buffer[arrLen] = '\0';

	fifoMsg.Add(buffer);
	AddingMessage = false;
}

void logger::printNextMsg(){
	char * buffer = (char *)fifoMsg.Get();
	if(buffer){
		stream=(streamDest)getLogChoice();
		if(stream==strNone) return;

		if(stream==strSerial || stream==strBoth)
			iprintf("%s", buffer);
		if(stream==strUdpLog || stream==strBoth){
			sendUdp(buffer);
		}
		bool freeMem = true;
		if(strcmp(buffer,OVERCALLMESSAGE) == 0){
			freeMem = false;
		}
		fifoMsg.MoveReader(freeMem);
	}
}

void logger::prnMsg(char * msg, int size, char id){
	char buffer[size+2];
	bool printAnyway=TRUE;

	stream=(streamDest)getLogChoice();
	if(stream==strNone) return;

	if(msg[0]==0x05 && size==1)
		printAnyway=FALSE;
	else if(msg[0]=='#' && msg[1]=='N' && msg[2]=='o' && msg[4]=='t' && msg[5]=='a' && msg[6]=='g' && size<14)
		printAnyway=FALSE;

	if(printMsg || printAnyway){
		buffer[1]=id;
		buffer[2]=' ';
		for(int i=0; i<size; i++){
			buffer[i+3]=msg[i];
		}
		buffer[0]='\n';
		buffer[size+3]='\0';
		if(stream==strSerial || stream==strBoth)
			iprintf(buffer);
		if(stream==strUdpLog || stream==strBoth)
			sendUdp(buffer);
	}
}

void logger::sendUdp(char * buffer){
	int portNumber = getUdpPort();
	IPADDR destIpAddress = AsciiToIp(getUdpIp());

	UDPPacket pkt;
	pkt.SetSourcePort(portNumber);
	pkt.SetDestinationPort(portNumber);
	pkt.AddData(buffer);
	pkt.AddDataByte(0);
	pkt.Send(destIpAddress);
}
