/*
 * Report2Server.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#include <constants.h>
#include <nbrtos.h>
#include <json_lexer.h>
#include <nbstring.h>
#include <webclient/http_funcs.h>
#include <libs/Report2Server.h>

bool sendingServer = false;

uint32_t TaskAllParamsStack[USER_TASK_STK_SIZE];

NBString nbs;

/****************************SEND THE JSON DATA THROUGH POST AT A WEB SERVICE********************************/
const char * urlProcessData =
		"http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiProcessData&milis=1587171056763&userId=Integra";
void SendProcessData(void *pd) {

	iprintf("reporting");

	sendingServer = true;

	NBString msg, onlytags = nbs;

	nbs = "";

	msg = (const char*) "#00 INTEGRA 101 00001\n";

	msg += onlytags.c_str();

	msg += (const char*) "#99 0000000223";

	ParsedJsonDataSet JsonOutObject;
	ParsedJsonDataSet JsonInObject;

	JsonOutObject.StartBuilding();

	JsonOutObject.Add("command", "integraAeiProcessData");

	JsonOutObject.Add("commandDateTime", "2010-12-08T18:09:45");

	JsonOutObject.Add("rawData", msg.c_str());

	JsonOutObject.DoneBuilding();

	bool result = DoJsonPost(urlProcessData, JsonOutObject, JsonInObject, NULL,
			10 * TICKS_PER_SECOND);

	if (result) {
		iprintf("Integra said [%s]\r\n",
				JsonInObject.FindFullNameString("info"));
	} else {
		iprintf("Result failed\r\n");
		nbs += onlytags;
	}
	sendingServer = false;
}

void Report(NBString tags) {
	nbs += tags + "\n";
	if (!sendingServer) {
		OSSimpleTaskCreatewName(SendProcessData, MAIN_PRIO + 2, "TaskSimple");
	}
}

const char * urlkeepalive =
		"http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiAlive&milis=1607468625781&userId=Integra";
ParsedJsonDataSet SendKeepAlive() {
	ParsedJsonDataSet JsonInObject;
	bool result = DoGet(urlkeepalive, JsonInObject, 10 * TICKS_PER_SECOND);

	if (result) {
		iprintf("Integra said [%s]\r\n",
				JsonInObject.FindFullNameString("responseCode"));
	} else {
		iprintf("Result failed\r\n");
	}
	return JsonInObject;
}

const char * urlMirror =
		"http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiMirrorData&milis=1607468997173&userId=Integra";
void SendMirrorData(char * msg[][2]) {
	ParsedJsonDataSet JsonOutObject;

	ParsedJsonDataSet JsonInObject;

	JsonOutObject.StartBuilding();

	JsonOutObject.Add(msg[0][0], msg[0][1]);

	JsonOutObject.DoneBuilding();

	bool result = DoJsonPost(urlMirror, JsonOutObject, JsonInObject, NULL);

	if (result) {
		printf("Integra said [%s] \r\n",
				JsonInObject.FindFullNameString("info"));
	} else {
		iprintf("Result failed\r\n");
	}
}

