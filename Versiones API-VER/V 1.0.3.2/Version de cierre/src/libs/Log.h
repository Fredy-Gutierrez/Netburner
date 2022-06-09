/*
 * SpeedProcess.h
 *
 *  Created on: 21 april 2021
 *      Author: Integra MGZ
*/
#ifndef LOGGER_H_
#define LOGGER_H_

#include <nbstring.h>

enum streamDest {strNone, strSerial, strUdpLog, strBoth };
#define OVERCALLMESSAGE "OverCall Message\0"

/***************************************************** Class log ***********************************************/
class logger {
private:
	streamDest stream;
	bool printMsg;
	bool AddingMessage;
public:
	logger();
	void setStream(int str);
	void setMsgPrn(bool prn);
	bool getMsgPrn();
	void prn(char * strToPrn);
	void newPrint(char * format, ...);
	void printNextMsg();
	void prnMsg(char * msg, int size, char id);
private:
	void sendUdp(char * buffer);

};

#endif /*LOGGER_H_*/

