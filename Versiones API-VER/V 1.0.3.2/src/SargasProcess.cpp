/*
 * SargasProcess.cpp
 *
 *  Created on: 7 oct 2021
 *      Author: mgz
 */

#include <stdio.h>
#include <tcp.h>
#include <iosys.h>
#include <utils.h>
#include <time.h>
#include <nbstring.h>
#include <string.h>
#include <stdlib.h>
#include <constants.h>
#include <libs/TagProcessor.h>
#include <string.h>
#include <syslog.h>
#include "libs/Configurations.h"
#include "libs/Tcp_Class.h"
#include "libs/Default_Values.h"
#include "libs/Report2Server.h"
#include "libs/Rtc_Controller.h"
#include "libs/Log.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"

/***************************************************** Class sargasProc **********************************************
class sargasProc {
public:
	sargasProc();
	void attend();
	void start();
	void stop();
	void noActive();
	bool getProc();
	bool getReading();
	void sargasRfOff();
	void sargasRfOn();
	void sargasGetDateTime();
	void init(char identifier, int qtyAntennas);
private:
	tyCmd commands[MAX_COMMANDS] = { {cmdNone, 0,0,0,0}, {cmdTurnOnAnt,onRf,6,7,1}, {cmdTurnOffAnt,offRf,6,7,1},
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
	char strPrn[500];

	//variables added for using two Sargas
	bool active;
	char id;
	char myAntNumber;
	int qtyAnt;

	void formatRxData(char * dataRx);
	void sendCmd();
	idResp dataRxInterpreter(char * dataRx, int qty, typeAeiTime * t);
	idResp checkOtherResp(char * dataRx, int qtyRx, idResp respToChek);
};
*/

sargasProc::sargasProc(){ // @suppress("Class members should be properly initialized")
	myAntNumber='0';
}

void sargasProc::init(char identifier, int qtyAntennas, bool stEndTag) {
	requestToStop=FALSE;
	requestToSendCmd=FALSE;
	status.srState=stSrIni;
	status.lastCmdSent=&commands[cmdNone];
	status.lastRespRx=respOther;
	status.antState=antUndefined;
	status.sincByUtrAei=FALSE;
	status.lastSincTime.sysTime=0;
	status.lastSincTime.msCurrSec=0;
	synchInit=FALSE;//TRUE FOR SARGAS UPDATE THE SYSTEM TIME
	id=identifier;
	if(id=='R')
		myAntNumber='1';
	else
		myAntNumber='0';
	qtyAnt=qtyAntennas;
	startEndTag = stEndTag;
}

void sargasProc::attend(){
	switch (status.srState){
		case stSrIni:
			OSTimeDly(4);  //200 mili
		break;

		case stSrStart:
			logg.newPrint("\nConnecting to Sargas%c with IP:port address: %s:%d", id, getSargasIp(id), DEFAULT_SARGAS_ADMIN_PORT);
			closeTcp(fd);
			fd=connectTcp(AsciiToIp(getSargasIp(id)), DEFAULT_SARGAS_ADMIN_PORT);//connects the tcp return >0 for success -1 for fail
			status.srState=stSrWaitCx;
		break;

		case stSrWaitCx:
			if(fd<0) {
				logg.newPrint("\nERROR: Connection to Sargas%c failed, return code: %d", id, fd);
				status.srState=stSrStart;
				stMac30.setStatus(idxTagReader, 'F', 12);
				OSTimeDly(TICKS_PER_SECOND*10); //Wait 10 seconds to retry
			}
			else {
				int qtyRx=ReadWithTimeout(fd, RxBuffer, RX_BUFSIZE, 20);//ret: 0 timeout=1 sec, -1 error fd, >0 qty bytes to read
				logg.newPrint("\n%c %s", id, RxBuffer);
				RxBuffer[7]='\0';
				if(qtyRx>80 && strcmp(RxBuffer,"#Sargas\0")==0){
					status.srState=stSrSynch;
					status.antState=antUndefined;
					cmdToSend=& commands[cmdGetTimeDate];
					stMac30.setStatus(idxTagReader, 'W', 14);
					OSTimeDly(cmdToSend->delayToSend);
				}
				else{ //Tue answer does not start with "#Sargas"
					logg.newPrint("\nERROR: Eco from Sargas%c was not OK", id);
					status.srState=stSrStart;
					stMac30.setStatus(idxTagReader, 'F', 13);
					OSTimeDly(TICKS_PER_SECOND*5); //Wait 5 seconds to retry
				}
			}
		break;

		case stSrSynch:
			status.lastCmdSent=cmdToSend;
			sendCmd();
			switch(status.lastRespRx){
				case respErrorCx:
				case respTxError:
					status.srState=stSrStart;
					stMac30.setStatus(idxTagReader, 'F', 13);
					break;

				case respTimeDate:
					cmdToSend=& commands[cmdTurnOffAnt];
					status.srState=stSrPoll;
					stMac30.setStatus(idxTagReader, 'N', 11);
					break;

				default:
					break;
			}
			OSTimeDly(cmdToSend->delayToSend);
		break;

		case stSrPoll:
			if(cmdToSend->id!=cmdNone){
				status.lastCmdSent=cmdToSend;
				sendCmd();
				if(status.lastRespRx==respErrorCx || status.lastRespRx==respTxError)
					status.srState=stSrStart;
				else{
					if(!requestToSendCmd)
						cmdToSend=& commands[cmdAskForTags];
					else{
						cmdToSend=& commands[cmdReqToSend];
						requestToSendCmd=FALSE;
					}
				}
				OSTimeDly(cmdToSend->delayToSend);
			}
			else{
				OSTimeDly(10); //Wait 500 mili
			}
		break;

		case stSrStandBy:
			OSTimeDly(TICKS_PER_SECOND); //Wait 1 sec
		break;

		case strSrNoActive:
		break;

	}
	if(requestToStop){
		logg.newPrint("\nStopping Sargas%c reader procees by user request", id);
		closeTcp(fd);
		requestToStop=FALSE;
		stMac30.setStatus(idxTagReader, 'W', 15);
		status.srState=stSrStandBy;
	}

}

void sargasProc::sendCmd(){
	int qtyTx, qtyRx;
	logg.prnMsg((char *)cmdToSend->msg, cmdToSend->qtyTx, id);
	qtyTx=write(fd, cmdToSend->msg, cmdToSend->qtyTx); //ret: 0 timeout, -1 error TCP, >0 qty bytes written
	if(qtyTx==cmdToSend->qtyTx){
		qtyRx=ReadWithTimeout(fd, RxBuffer, RX_BUFSIZE, 20);//ret: 0 timeout=1 sec, -1 error fd, >0 qty bytes to read
		if(qtyRx>0){
			logg.prnMsg(RxBuffer, qtyRx, id);
			getNowTime(&timeRx);
			status.lastRespRx=dataRxInterpreter(RxBuffer, qtyRx, & timeRx);
		}
		else if(qtyRx==0){
			logg.newPrint("\nTIMEOUT: while waiting for a message from Sargas%c", id);
			status.lastRespRx=respTimeOut;
		}
		else{
			logg.newPrint("\nERROR: TCP error while waiting for a message from Sargas%c", id);
			status.lastRespRx=respErrorCx;
		}
	}
	else{ //Any other result from write
		logg.newPrint("\nERROR: TCP error while transmitting a message to Sargas%c", id);
		status.lastRespRx=respTxError;
	}
}


idResp sargasProc::dataRxInterpreter(char * dataRx, int qty, typeAeiTime * t){
	idResp respRx=respOther;
	for(int i=0; i<(MAX_RESPONSES-1); i++){
		int difSize=qty-responses[i].size;
		if(difSize>=0){
			if(strncmp(dataRx, responses[i].msg, responses[i].size)==0){
				respRx=responses[i].id;
				//sprintf(strPrn, "\nRespuesta %d %d bien", i, respRx); logg.prn(strPrn);
				break;
			}
		}
	}

	//sprintf(strPrn, "\nRespuesta %d recibida", respRx); logg.prn(strPrn);

	switch(status.lastCmdSent->id){
		case cmdNone: default:
			break;
		case cmdTurnOnAnt0:
		case cmdTurnOnAnt1:
		case cmdTurnOnAntB:
			if(respRx==respDone){
				status.antState=antON;
			}
			else{
				status.antState=antUndefined;
			}
			break;
		case cmdTurnOffAnt:
			if(respRx==respDone){
				status.antState=antOFF;
			}
			else{
				status.antState=antUndefined;
			}
			break;
		case cmdAskForTags:
			if(respRx!=respOther){
				break;
			}
			respRx=checkOtherResp(dataRx, qty, respTagHex);
			if(respRx==respTagHex){
				uint32_t speed=speedDmh.getValue();
				int direction=speedDmh.getDir();
				char asciiTime[30];
				/* This code is used to save data with Sargas time
				typeAeiTime tRx;
				getSargasTime(dataRx, & tRx, 32, 33);
				convertTimeToASCII(& tRx, asciiTime);
				*/
				//Save data with MAC30 time when message was received
				convertTimeToASCII(t, asciiTime);
				dataRx[63] = '\0';
				dataRx[53] = ' ';
				logg.newPrint("\n%c. %s  %s, speed=%f,  dir=%d", id, asciiTime, dataRx, ((float)speed)/10, direction);
				if(formatRxData(dataRx)){
					logg.newPrint("\n%c. %s  %s, speed=%f,  dir=%d. Antenna # changed if required", id, asciiTime, dataRx, ((float)speed)/10, direction);
					saveHexTag(dataRx, t, speed, direction);
				}
				else{
					/*when there is only one antenna and the antenna number
					 * with which the tag was read is not equal to
					 * the antenna number with which it was configured*/
					logg.newPrint(" ... Not saved");
				}
			}
			break;
		case cmdStayAlive:
			if(respRx==respDone){

			}
			else{

			}
			break;
		case cmdSetDate:
		case cmdSetTime:
			if(respRx==respDone){
				status.sincByUtrAei=TRUE;
				getNowTime(& status.lastSincTime);
			}
			else{
				status.sincByUtrAei=FALSE;
			}
			break;
		case cmdGetTimeDate:
			respRx=checkOtherResp(dataRx, qty, respTimeDate);
			if(respRx==respTimeDate){
				typeAeiTime t;
				char asciiTime[30];
				dataRx[qty-1]='\0';
				getSargasTime(dataRx, & t, 0, 0);
				if(synchInit){
					setSystemTime(& t);//We will use the RTC TIME
					synchInit=FALSE;
				}
				getNowTimeAscii(asciiTime);
				logg.newPrint("\nSargas%c time %s, system time %s", id, dataRx, asciiTime);
			}
			break;
	}
	return respRx;
}

idResp sargasProc::checkOtherResp(char * dataRx, int qtyRx, idResp respToChek){
	idResp respRx=respOther;
	if(qtyRx>=responses[respToChek].size){
		int j=0;
		for( ; j<responses[respToChek].sizeHex; j++){
			if(dataRx[(int)responses[respToChek].msgIndex[j]]!=responses[respToChek].msg[j]){
				logg.newPrint("\n%c Data index %d not ok, data expected=%c, data received=%c", id, j, responses[respToChek].msg[j], dataRx[(int)responses[respToChek].msgIndex[j]]);
				break;
			}
		}
		if(j==responses[respToChek].sizeHex){
			//sprintf(strPrn, "\nHex resp"); logg.prn(strPrn);
			respRx=respToChek;
		}
		else{
			logg.newPrint("\n%c No hex response %d", id, j);
		}
	}
	return respRx;
}


/* public functions  */

void sargasProc::start() {
	if(status.srState==stSrIni || status.srState==stSrStandBy)
		status.srState=stSrStart;
}

void sargasProc::stop() {
	requestToStop=TRUE;
}

void sargasProc::noActive() {
	status.srState=strSrNoActive;
}

bool sargasProc::getProc(){
	if(status.srState==stSrPoll)
		return TRUE;
	else
		return FALSE;
}

bool sargasProc::getReading(){
	if(status.antState==antON)
		return TRUE;
	else
		return FALSE;
}

void sargasProc::rfOff() {
	if(status.srState==stSrPoll){
		cmdReqToSend=cmdTurnOffAnt;
		requestToSendCmd=TRUE;
		if(startEndTag)
			addStartEndTag(FALSE);
	}
}

/*****************************************TURNS OFF THE SARGAS'S RF******************************/
void sargasProc::rfOn() {
	if(status.srState==stSrPoll){
		cmdReqToSend=cmdTurnOnAntB;
		requestToSendCmd=TRUE;
		speedDmh.reset();
		if(startEndTag)
			addStartEndTag(TRUE);
	}
}

void sargasProc::getDateTime() {
	if(status.srState==stSrPoll){
		cmdReqToSend=cmdGetTimeDate;
		requestToSendCmd=TRUE;
	}
}

bool sargasProc::formatRxData(char * dataRx){
	if(getQtySargas()==1)
		return TRUE;

	if(qtyAnt==1 && dataRx[54]!=myAntNumber){
		return FALSE;
	}
	dataRx[54]=myAntNumber;
	return TRUE;
}

sargasProc sargasR;
sargasProc sargasL;

