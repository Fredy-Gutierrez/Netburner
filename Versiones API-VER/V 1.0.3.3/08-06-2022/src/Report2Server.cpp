/*
 * Report2Server.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include <predef.h>
#include <ctype.h>
#include <dhcpclient.h>
#include <constants.h>
#include <iointernal.h>
#include <startnet.h>
#include <time.h>
#include <nbrtos.h>
#include <system.h>
#include <stdlib.h>
#include <stdio.h>
#include <udp.h>
#include <utils.h>
#include <crypto/ssl.h>
#include <dns.h>
#include <fdprintf.h>
#include <http.h>
#include <tcp.h>
#include <json_lexer.h>
#include <libs/TagProcessor.h>
#include <libs/TagUtilities.h>
#include <webclient/http_funcs.h>
#include <webclient/web_buffers.h>
#include <webclient/web_client.h>

#include "libs/Log.h"
#include "libs/Configurations.h"
#include "libs/Rtc_Controller.h"
#include "libs/Default_values.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"
#include "libs/Report2Server.h"
#include "libs/fifo/Fifo.h"
#include "libs/Train.h"
#include "libs/DBTrainDriver.h"

extern logger logg;

enum ServerState{StInit, StWait, StSendStatus, StAskForTrain, StSendTrain, StAfterSend, StCheckNet, StWaitRestart};
ServerState state;

int ticksToSendStatus;

ParsedJsonDataSet JsonOutObject;
ParsedJsonDataSet JsonInObject;

typeTrain bufferTrainRead;//

SDTrainUsers trainDBUserR;

/*********************************MAKES A READ/OVERWRITE REQUEST TO DBTRAIN**********************
 * 		return			->		true for request success
 * 								false for request fail
 * */
bool makeRequest(RequestType request){
	trainDBUserR.request = request;
	trainDBUserR.dataBuffer = &bufferTrainRead;
	trainDBUserR.useTrainControl = true;
	trainDBUserR.dataType = TrainData;

	if(request == ReadRequest){
		trainDBUserR.modReqStFlag = true;
	}else if(request == OverWriteRequest){
		trainDBUserR.modReqStFlag = false;
	}else{
		return false;
	}

	return newRequest(&trainDBUserR);
}

char urlServer[255];
const char * getUrl(char * command){
	memset(urlServer,'\0',255);
	strcpy(urlServer, getServerUrl());
	strcat(urlServer, "?action=aeiCommand&command=");
	strcat(urlServer, command);
	strcat(urlServer, "&milis=");
	strcat(urlServer, getUsTime());
	strcat(urlServer, "&userId=Integra");
	return urlServer;
}

void timeEoc(typeT94Eoc * carToTime, typeT94EocTimed * carTimed){
	char temp[30];

	memcpy(carTimed, carToTime, sizeof(carTimed->segmentEoc));
	convertTimeToASCII(& carToTime->startTime, temp);
	for(int i=0; i<23; i++)
		carTimed->timeIni[i]=temp[i];
	convertTimeToASCII(& carToTime->endTime, temp);
	for(int i=0; i<23; i++)
		carTimed->timeEnd[i]=temp[i];
	carTimed->separator[0]='&';
	carTimed->tail[0]='\0';
}

void timeEot(typeT94Eot * carToTime, typeT94EotTimed * carTimed){
	char temp[30];

	memcpy(carTimed, carToTime, sizeof(carTimed->segmentEot));
	convertTimeToASCII(& carToTime->startTime, temp);
	for(int i=0; i<23; i++)
		carTimed->timeIni[i]=temp[i];
	convertTimeToASCII(& carToTime->endTime, temp);
	for(int i=0; i<23; i++)
		carTimed->timeEnd[i]=temp[i];
	carTimed->separator[0]='&';
	carTimed->tail[0]='\0';
}

void timeACar(typeT94Rre * carToTime, typeT94RreTimed * carTimed){
	char temp[30];

	memcpy(carTimed, carToTime, sizeof(carTimed->segmentRre));
	convertTimeToASCII(& carToTime->startTime, temp);
	for(int i=0; i<23; i++)
		carTimed->timeIni[i]=temp[i];
	convertTimeToASCII(& carToTime->endTime, temp);
	for(int i=0; i<23; i++)
		carTimed->timeEnd[i]=temp[i];
	carTimed->separator[0]='&';
	carTimed->tail[0]='\0';
}

bool getSegFmted(char * pSegToPost, void * trainSeg){
	bool readOk=FALSE;
	char * seg;

	seg=(char *)trainSeg;
	if(seg[0]=='R'){
		typeT94RreTimed carT94ReeTimed;
		typeT94Rre * carT94Ree;
		carT94Ree=(typeT94Rre *)seg;
		timeACar(carT94Ree, & carT94ReeTimed);
		memcpy(pSegToPost, & carT94ReeTimed, sizeof(typeT94RrePost));

		pSegToPost[sizeof(typeT94RrePost)]=0;
		readOk=TRUE;
	}
	else if(seg[0]=='A'){
		memcpy(pSegToPost, seg, sizeof(typeT94AemPost));
		pSegToPost[sizeof(typeT94AemPost)]=0;
		readOk=TRUE;
	}
	else if(seg[0]=='E'){
		if(seg[2]=='C'){
			typeT94EocTimed eocTimed;
			typeT94Eoc * carT94Eoc;
			carT94Eoc = (typeT94Eoc *) seg;

			timeEoc(carT94Eoc, &eocTimed);
			memcpy(pSegToPost, &eocTimed, sizeof(typeT94EocPost));

			pSegToPost[sizeof(typeT94EocPost)]=0;
			readOk=TRUE;
		}
		else if(seg[2]=='T'){
			typeT94EotTimed eotTimed;
			typeT94Eot * carT94Eot;
			carT94Eot = (typeT94Eot *) seg;

			timeEot(carT94Eot, &eotTimed);
			memcpy(pSegToPost, &eotTimed, sizeof(typeT94EotPost));

			pSegToPost[sizeof(typeT94EotPost)]=0;
			readOk=TRUE;
		}
	}

	return readOk;
}


bool sendStatus() {
	char time_date[24];
	char postObjStr[1024];
	memset(postObjStr,'\0',1024);

	getNowTimeAscii(time_date);
	time_date[23] = '\0';

	JsonOutObject.StartBuilding();
	JsonOutObject.Add("action", "aeiCommand");
	JsonOutObject.Add("command", "integraAeiProcessLogData");
	JsonOutObject.Add("commandDateTime", time_date);
	JsonOutObject.Add("rawData",stMac30.getStatus());
	JsonOutObject.Add("userId", "Integra");
	JsonOutObject.DoneBuilding();
	logg.newPrint("\nStatus out:   ");
	JsonOutObject.PrintObjectToBuffer(postObjStr, 1024);
	logg.newPrint(postObjStr);

	bool result = DoJsonPost(getUrl("integraAeiProcessLogData"), postObjStr, JsonInObject, NULL, TICKS_PER_SECOND * 15);

	logg.newPrint("\nStatus in:   ");
	if(result){
		memset(postObjStr,'\0',1024);
		JsonInObject.PrintObjectToBuffer(postObjStr, 1024);
		logg.newPrint(postObjStr);
	}
	return result;
}

enum TrainSendState{StSendInit, StSendAem, StSendRre, StSendEoc, StSendEot, StSendEnd};

/*********************************FUNTION THAT SEND A COMPLETE TRAIN *********************************
 * 		return			->			true for train completely send
 * 									false for train couldn´t send
 **/
int rreSendCounter = 0;
bool sendTrain() {
	typeTrain * train = &bufferTrainRead;//get the next train to send from sd card
	char postObjStr[1024];

	bool TrainSendResult = false;//if train could be send
	TrainSendState sendState;//control the next train segment

	switch (train->stateOfReport){
		case stNotReady:
		case stReady:
			rreSendCounter = 0;
			sendState = StSendInit;
			break;
		case stSegAem:
			sendState = StSendAem;
			break;
		case stSegRre:
			sendState = StSendRre;
			break;
		case stSegEot:
			sendState = StSendEot;
			break;
		case stSegEoc:
			sendState = StSendEoc;
			break;
		case stReported:
			sendState = StSendEnd;
			TrainSendResult = true;
			break;
		default:
			sendState = StSendInit;
			rreSendCounter = 0;
			break;
	}
	//int rreSendCounter = 0;//control rres sent

	while(sendState != StSendEnd){//until train has been completely send or couldn´t send
		char msgSeg[100];//message segment converted to char array
		bool result = false;//converted segment result
		/************GET THE NEXT TRAIN SEGMENT INTO CHAR BUFFER TO SEND**********/
		switch(sendState){
			case StInit:
				sendState = StSendAem;
				break;
			case StSendAem:
				train->stateOfReport = stSegAem;
				result = getSegFmted(msgSeg, (void *)&train->segAem);
				sendState = StSendRre;
				break;
			case StSendRre:{
				train->stateOfReport = stSegRre;
				if(rreSendCounter < train->qtyRre){
					result = getSegFmted(msgSeg, (void*)&train->segRre[rreSendCounter]);
					rreSendCounter++;
				}else{
					sendState = StSendEot;
				}
				break;
			}
			case StSendEot:
				train->stateOfReport = stSegEot;
				result = getSegFmted(msgSeg, (void *)&train->segEot);
				sendState = StSendEoc;
				break;
			case StSendEoc:
				train->stateOfReport = stSegEoc;
				result = getSegFmted(msgSeg,(void *)&train->segEoc);
				sendState = StSendEnd;
				TrainSendResult = true;
				break;
			default:
				sendState = StSendEnd;
		}
		/************IF SEGMENT COULD BE CONVERTED, MAKE THE POST**********/
		if(result){
			//NBString msg="";//post rawData
			char counter[6], length[11], time_date[24], rawData[200];//car counter, message length, system date time
			memset(rawData,'\0',200);

			getNowTimeAscii(time_date);
			time_date[23] = '\0';

			/************************BUILD JSON*****************/
			sprintf(counter, "%05d", getCarCounter());//convert an integer to string of 5 digits
			//sprintf(length, "%010d", (int)msg.length());
			//msg = (const char*) "#00 "+(NBString)getAeiID()+" 102 "+(NBString)time_date+" "+(NBString)counter+"\r\n"+(NBString)msgSeg+ (const char *)"\r\n#99 " +(NBString)length;

			strcpy(rawData, "#00 " ); strcat(rawData, getAeiID() ); strcat(rawData, " 102 " ); strcat(rawData, time_date ); strcat(rawData, " " );
			strcat(rawData, counter ); strcat(rawData, "\r\n" ); strcat(rawData, msgSeg ); strcat(rawData, "\r\n#99 ");

			sprintf(length, "%010d", (int)strlen(rawData) );
			length[10] = '\0';

			strcat(rawData, length);

			JsonOutObject.StartBuilding();
			JsonOutObject.Add("command", "integraAeiProcessData");
			JsonOutObject.Add("commandDateTime", time_date);
			//JsonOutObject.Add("rawData", msg.c_str());
			JsonOutObject.Add("rawData", rawData);
			JsonOutObject.DoneBuilding();
			logg.newPrint("\nPost out:   ");
			memset(postObjStr,'\0',1024);
			JsonOutObject.PrintObjectToBuffer(postObjStr, 1024);
			logg.newPrint(postObjStr);

			/************************POST JSON*****************/
			int retries = 0;
			bool PostResult = DoJsonPost( getUrl("integraAeiProcessLogData"), postObjStr, JsonInObject, NULL, 15 * TICKS_PER_SECOND );
			while( !PostResult && ++retries < 3 ){/************************UNTIL THE POST SUCCESS OR 3 RETRIES*****************/
				OSTimeDly(15 * TICKS_PER_SECOND);
				PostResult = DoJsonPost( getUrl("integraAeiProcessLogData"), postObjStr, JsonInObject, NULL, 15 * TICKS_PER_SECOND );
			}
			logg.newPrint("\nPost in:   ");
			if(PostResult){//IF SEGMENT HAS BEEN SENT
				incrementCarCounter();//increments the car global counter
				memset(postObjStr,'\0',1024);
				JsonInObject.PrintObjectToBuffer(postObjStr, 1024);//To print the post result into a buffer
				logg.newPrint(postObjStr);//To print the post result in logger
			}else{//IF SEGMENT COULDN´T BE SENT
				TrainSendResult = false;//THE TRAIN SEND FAIL
				if(sendState == StSendRre)//if we don´t have server response, and we are sending a RRE we going to decrement the rreSendCounter
					rreSendCounter--;
				sendState = StSendEnd;//CLOSE WHILE
			}
		}
	}
	if(TrainSendResult){//THE TRAIN HAS BEEN COMPLETELY SENT
		train->reportedFlag = true;
		train->stateOfReport = stReported;
	}
	return TrainSendResult;
}

/****************************SENDS THE JSON DATA THROUGH POST AT A WEB SERVICE********************************/
void SendProcessData(void *pd) {
	IPADDR ipAddress=IPADDR::NullIP();

	int retries=0;
	int ticksToReq = 0;
	bool netAvailable = true;
	bool serverAvailable = true;

	while(true){
		switch(state){
			case StInit:
				OSTimeDly(TICKS_PER_SECOND * 15);
				if( getSychStatus() ){//Wait until system time is update by NTP
					logg.newPrint("\nURL: %s", getUrl("integraAeiProcessData"));
					ticksToSendStatus = 590;
					state = StWait;
				}
				break;
			case StWait:
				OSTimeDly(10);//10 for 500ms
				if(++ticksToSendStatus >= 600 && state == StWait){		//600 FOR 5 minutes - 200 FOR 1 minutes
					ticksToSendStatus=0;
					retries=0;
					state = StSendStatus;
				}else if(trainDBUserR.stateRequest == StNoRequest){	//When there is not train in buffer
					if(++ticksToReq >= 240){//	and have passed 240 ticks	(2 minutes)
						state = StAskForTrain;//	moves to ask for train
						ticksToReq = 0;
					}
				}else if(trainDBUserR.stateRequest == StCompletedReq && netAvailable && serverAvailable){	//There is a train to send and Ethernet connection and server are available
					ticksToReq = 0;
					retries=0;
					state = StSendTrain;
				}
				break;
			case StSendStatus://Not modified
				if(!sendStatus()){
					if(++retries>=3){
						state = StCheckNet;
						OSTimeDly(2);
					}
					else
						OSTimeDly(TICKS_PER_SECOND * 10);
				}
				else{
					serverAvailable = true;
					netAvailable = true;
					state = StWait;
					OSTimeDly(2);
				}
				break;
			case StAskForTrain:
				/*********************************MAKE A REQUEST, IF DB COULDN´T ACCEPT REQUEST WAIT UNTIL IT CAN***************************************/
				if(makeRequest(ReadRequest)){
					state = StWait;//process state changed to wait
				}else{//IF DB COULDN´T ACCEPT REQUEST WAIT
					OSTimeDly(TICKS_PER_SECOND);
				}
				break;
			case StSendTrain:
				/*********************************SEND NEXT TRAIN ONLY WHEN DB RESPONSE THE REQUEST***************************************/
				if(!bufferTrainRead.reportedFlag){
					if( sendTrain() ){
						trainDBUserR.stateRequest = StNoRequest;
						state = StAfterSend;
						logg.newPrint("\nFinish posting cars");
					}else{
						if(++retries > 3){
							/*********************************WHAT HAPPEND WHEN CAN´T SENT?***************************************/
							retries = 0;
							state = StCheckNet;
						}
						OSTimeDly(TICKS_PER_SECOND*30);
					}
				}else{
					trainDBUserR.stateRequest = StNoRequest;
					state = StWait;//process state changed to wait
				}
				break;
			case StAfterSend:
				/*********************************MAKES A NEW REQUEST TO OVERWRITE THE SENT TRAIN***************************************/
				if(makeRequest(OverWriteRequest)){
					trainDBUserR.stateRequest = StNoRequest;
					state = StWait;
				}else{
					OSTimeDly(10);
				}
				break;
			case StCheckNet:{//Not modified
				logg.newPrint("\nElapsed 20 minutes without server response to status post, checking Internet connection");
				int rv=GetHostByName(DEFAULT_NTP_SERVER_NAME, &ipAddress, INADDR_ANY, TICKS_PER_SECOND * 10);
				if(rv==DNS_OK || rv==DNS_NOSUCHNAME){
					if(!netAvailable){
						state = StWaitRestart;
						netAvailable = true;
					}else{
						logg.newPrint("\nInternet is available but server is out of service or router must be restart");
						serverAvailable = false;
						state = StWait;
					}
				}
				else{
					logg.newPrint("\nInternet not available continue waiting");
					state = StWait;
					netAvailable = false;
				}
				OSTimeDly(1);
				break;
			}
			case StWaitRestart://Not modified just for restart DHCP
				logg.newPrint("\nInternet connection available, MAC30 will be restarted the DHCP");
				state = StWait;
				restartDHCP();
				OSTimeDly(20);
				break;
		}
	}
}

/****************************MAKES A TASK TO SEND TO SERVER (IF THERE IS A TASK WHICH IS SENDING TO SERVER IT WILL SAVE THE TAGS IN A TEMPORAL VAR)********************************/
void initRep2Server() {
	state = StInit;
	trainDBUserR.stateRequest = StNoRequest;
	OSSimpleTaskCreatewName(SendProcessData, REPORT_SERVER_PRIO, "TaskSimple");
}




