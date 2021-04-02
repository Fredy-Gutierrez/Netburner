/*
 * Report2Server.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include "libs/Report2Server.h"
#include <constants.h>
#include <time.h>
#include <nbrtos.h>
#include <json_lexer.h>
#include <nbstring.h>
#include <webclient/http_funcs.h>
#include "libs/Configurations.h"

#include <system.h>
#include <stdlib.h>
#include <udp.h>
#include <utils.h>

bool sendingServer = false;//takes control of send's process
uint32_t TaskAllParamsStack[USER_TASK_STK_SIZE];//task stack
NBString tagContainer;//Save the tag's data to send
NBString base_Url;//Base server url


/*************************************************SENDS DATA THROUGH UDP*****************************************************/
void sendUdpPacket(NBString buffer){
	int portNumber = getUdpPort();
	IPADDR destIpAddress = AsciiToIp( getUdpIp() );

	UDPPacket pkt;
	pkt.SetSourcePort(portNumber);
	pkt.SetDestinationPort(portNumber);
	pkt.AddData(buffer.c_str());
	pkt.AddDataByte(0);
	pkt.Send(destIpAddress);
}


void getBaseUrl(){
	base_Url = (NBString)getServerUrl();
}

void setNewLocalBaseUrl(char * newurl){
	base_Url = (NBString)newurl;
}

NBString getTime(){
	time_t timeT;          // Stores the system time in time_t format
	struct tm stmLocal;    // Stores local time as struct tm
	timeT = time(NULL);   // Get system time as time_t
	localtime_r(&timeT, &stmLocal);

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
	sprintf(cy, "%d", stmLocal.tm_year+1900);

	NBString datehour = (const char*)""+(NBString)cy+"-"+(NBString)cmo+"-"+(NBString)cd+"T"+(NBString)ch+":"+(NBString)cm+":"+(NBString)cs+".000";

	return datehour;
}


/****************************SENDS THE JSON DATA THROUGH POST AT A WEB SERVICE********************************/
void SendProcessData(void *pd) {
	NBString urlProcessData = base_Url.c_str() + (NBString)"?action=aeiCommand&aeiId=INTEGRA&command=integraAeiProcessData&milis=1587171056763&userId=Integra";

	sendingServer = true;

	while(sendingServer){

		NBString msg, onlytags = tagContainer.c_str();

		tagContainer = "";

		NBString datehour = getTime();

		char counter[5];
		sprintf(counter, "%05d", getTagCounter());//convert an int to string of 5 digits

		/****************************************************START THE REPORT MSG CREATION************************************************/
		msg = (const char*) "#00 INTEGRA 101 "+datehour+" "+(NBString)counter+"\r\n";//THAT MUST BE A COUNTER VAL

		msg += onlytags.c_str();

		msg += (const char*) "#99 ";

		int bytesMsg = static_cast<int>(msg.length());//get the actual size from the msg tag to report

		char bytesChar[10];
		sprintf(bytesChar, "%010d", bytesMsg);

		msg += (const char*) ""+(NBString)bytesChar;//SIZE_OF
		/****************************************************ENDS THE REPORT MSG CREATION************************************************/

		ParsedJsonDataSet JsonOutObject;

		ParsedJsonDataSet JsonInObject;

		JsonOutObject.StartBuilding();

		JsonOutObject.Add("command", "integraAeiProcessData");

		JsonOutObject.Add("commandDateTime",datehour.c_str());

		JsonOutObject.Add("rawData", msg.c_str());

		JsonOutObject.DoneBuilding();

		sendUdpPacket((const char*) ""+getTime()+(NBString)", "+ msg + (NBString)"\n");

		bool result = DoJsonPost(urlProcessData.c_str(), JsonOutObject, JsonInObject, NULL,
				10 * TICKS_PER_SECOND);

		if (result) {
			incrementTagCounter();//incremente el contador global de tags

			iprintf("Integra said [%s]\r\n",
			JsonInObject.FindFullNameString("info"));

			sendUdpPacket((const char*)"" +getTime()+ ", Server Response: " +(NBString)JsonInObject.FindFullNameString("info")+(NBString)"\n");

		} else {
			iprintf("Result failed\r\n");
			tagContainer = onlytags + tagContainer;
		}

		if(tagContainer.empty()){
			sendingServer = false;
		}
	}
}


/****************************CHECKS THE SERVER'S STATUS********************************/
ParsedJsonDataSet SendKeepAlive() {

	NBString urlkeepalive = base_Url.c_str() + (NBString)"?action=aeiCommand&aeiId=INTEGRA&command=integraAeiAlive&milis=1607468625781&userId=Integra";

	ParsedJsonDataSet JsonInObject;
	bool result = DoGet(urlkeepalive.c_str(), JsonInObject, 10 * TICKS_PER_SECOND);

	if (result) {
		iprintf("Integra said [%s]\r\n",
				JsonInObject.FindFullNameString("responseCode"));
	} else {
		iprintf("Result failed\r\n");
	}
	return JsonInObject;
}


/****************************CHECKS THE DATA SENT AT SERVER********************************/
void SendMirrorData(char * msg[][2]) {

	NBString urlMirror = base_Url.c_str() + (NBString)"?action=aeiCommand&aeiId=INTEGRA&command=integraAeiMirrorData&milis=1607468997173&userId=Integra";

	ParsedJsonDataSet JsonOutObject;

	ParsedJsonDataSet JsonInObject;

	JsonOutObject.StartBuilding();

	JsonOutObject.Add(msg[0][0], msg[0][1]);

	JsonOutObject.DoneBuilding();

	bool result = DoJsonPost(urlMirror.c_str(), JsonOutObject, JsonInObject, NULL);

	if (result) {
		printf("Integra said [%s] \r\n", JsonInObject.FindFullNameString("info"));
	} else {
		iprintf("Result failed\r\n");
	}
}


/****************************MAKES A TASK TO SEND TO SERVER (IF THERE IS A TASK WHICH IS SENDING TO SERVER IT WILL SAVE THE TAGS IN A TEMPORAL VAR)********************************/
void Report(NBString tags) {

	sendUdpPacket((const char*) ""+getTime()+(NBString)", "+tags+(NBString)"\n");

	if(base_Url.empty()){
		getBaseUrl();
	}

	tagContainer += tags + "\n";

	if(!sendingServer){
		OSSimpleTaskCreatewName(SendProcessData, MAIN_PRIO + 2, "TaskSimple");
	}
}



