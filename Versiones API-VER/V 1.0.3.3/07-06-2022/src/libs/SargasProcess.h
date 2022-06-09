/*
 * SargasProcess.h
 *
 *  Created on: 7 october 2021
 *      Author: Integra MGZ
*/
#ifndef LIBS_SARGASPROCESS_H_
#define LIBS_SARGASPROCESS_H_

#include "libs/Rtc_Controller.h"
#include <basictypes.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>


//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */

const char onAnt0[] = {0x23, 0x61, 0x6E, 0x74, 0x65, 0x6E, 0x6E, 0x61, 0x2E, 0x65, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x30, 0x0D};
const char onAnt1[] = {0x23, 0x61, 0x6E, 0x74, 0x65, 0x6E, 0x6E, 0x61, 0x2E, 0x65, 0x6E, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x31, 0x0D};
const char onAntB[] = {0x23, 0x36, 0x34, 0x30, 0x31, 0x0D};
const char offRf[] = {0x23, 0x36, 0x34, 0x30, 0x30, 0x0D};
const char askForTags[] = {0x05 };
const char stayAlive[] = {0x23, 0x61, 0x72, 0x65, 0x2E, 0x79, 0x6F, 0x75, 0x2E, 0x61, 0x6C, 0x69, 0x76, 0x65, 0x0D};
const char setDate[] = {0x23, 0x62, 0x60, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x2F, 0x00, 0x00, 0x0D};
const char setTime[] = {0x23, 0x62, 0x61, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x3A, 0x00, 0x00, 0x0D};
const char getTime[] = {0x23, 0x32, 0x32, 0x0D};
const char rError[] = {'#', 'E', 'r', 'r', 'o', 'r', 0x0D};
const char rDone[] = {'#', 'D', 'o', 'n', 'e', 0x0D};
const char rNoTag[] = {'#', 'N', 'o', ' ', 't', 'a', 'g', ' ', 'd', 'a', 't', 'a', 0x0D};
const char rTagHex[] = {'#', '&', ':', ':', '.', ' ', '/', '/', '%', '-', '*', '-', 0x0D};
const char rTagHexIndex[] = {0, 31, 34, 37, 40, 44, 47, 50, 53, 55, 58, 59, 63};
const char rTimeDateHex[] = {':', ':', '.', ' ', '/', '/', 0x0D};
const char rTimeDateHexIndex[] = {2, 5, 8, 11, 14, 17, 20};

enum stSargas {stSrIni, stSrStart, stSrWaitCx, stSrSynch, stSrPoll, stSrStandBy, strSrNoActive};
enum stAnt {antUndefined, antON, antOFF};

enum idCmd {cmdNone, cmdTurnOnAnt0, cmdTurnOnAnt1, cmdTurnOnAntB, cmdTurnOffAnt, cmdAskForTags, cmdStayAlive, cmdSetDate, cmdSetTime, cmdGetTimeDate};
typedef struct {
	idCmd id;
	const char * msg;
	int qtyTx;
	int qtyRx;
	uint32_t delayToSend;
}tyCmd;
#define MAX_COMMANDS		10

enum idResp {respError, respDone, respNoTag, respTagHex, respTimeDate, respErrorCx, respTimeOut, respTxError, respOther, respNone};
typedef struct {
	idResp id;
	const char * msg;
	int size;
	int sizeHex;
	const char * msgIndex;
}tyResp;
#define MAX_RESPONSES		5

typedef struct{
	tyCmd * lastCmdSent;
	idResp lastRespRx;
	stSargas srState;
	stAnt antState;
	bool sincByUtrAei;
	typeAeiTime lastSincTime;
}typeStatusSargas;

#define RX_BUFSIZE (499)//buff size for data received through tcp

#define sargasLeft		0
#define sargasRight		1
#define sargasBoth		2

/***************************************************** Class sargasProc **********************************************
*/
class sargasProc {
public:
	sargasProc();
	void attend();
	void start();
	void stop();
	void noActive();
	bool getProc();
	bool getReading();
	void rfOff();
	void rfOn();
	void getDateTime();
	void init(char identifier, int qtyAntennas, bool stEndTag = FALSE);
private:
	tyCmd commands[MAX_COMMANDS] = { {cmdNone, 0,0,0,0}, {cmdTurnOnAnt0,onAnt0,18,7,1}, {cmdTurnOnAnt1,onAnt1,18,7,1},
			{cmdTurnOnAntB,onAntB,6,7,1}, {cmdTurnOffAnt,offRf,6,7,1},
			{cmdAskForTags,askForTags,1,64,1}, {cmdStayAlive,stayAlive,15,6,100}, {cmdSetDate,setDate,12,7,1},
			{cmdSetTime,setTime,12,7,1}, {cmdGetTimeDate,getTime,4,22,20}};
	tyResp responses[MAX_RESPONSES] = { {respError, rError, 7, 0, NULL}, {respDone, rDone, 6, 0, NULL}, {respNoTag, rNoTag, 13, 0, NULL}, {respTagHex, rTagHex, 64, 13, rTagHexIndex},
			{respTimeDate, rTimeDateHex, 21, 7, rTimeDateHexIndex}};

	tyCmd * cmdToSend;
	bool requestToStop;
	bool requestToSendCmd;
	typeStatusSargas status;
	bool synchInit;
	idCmd cmdReqToSend;
	int fd; //file descriptor which controls the tcp
	char RxBuffer[RX_BUFSIZE];
	typeAeiTime timeRx;

	//variables added for using two Sargas
	bool active;
	char id;
	char myAntNumber;
	int qtyAnt;
	bool startEndTag;

	bool formatRxData(char * dataRx);
	void sendCmd();
	idResp dataRxInterpreter(char * dataRx, int qty, typeAeiTime * t);
	idResp checkOtherResp(char * dataRx, int qtyRx, idResp respToChek);
};

extern sargasProc sargasR;
extern sargasProc sargasL;

#endif /* LIBS_SARGASPROCESS_H_ */
