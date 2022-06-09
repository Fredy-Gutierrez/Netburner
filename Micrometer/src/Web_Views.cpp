/*
 * Web_Views.cpp
 *
 *  Created on: 22 abr 2021
 *      Author: Integra Fredy
 */

#include <http.h>
#include <iosys.h>
#include <httppost.h>
#include <fdprintf.h>
#include "headers/Business.h"
#include "headers/Configurations.h"
#include "headers/Default_Values.h"
#include <nbstring.h>

/************************************************************GET FUNCTIONS FOR INPUTS CONFIG***************************************************************/
void WebHMaker(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getHMaker());
}
void WebHType(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getHType());
}
void WebHSerialNum(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getHSerialNum());
}

void WebDOperator(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDOperator());
}
void WebDSubStation(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDSubStation());
}
void WebDDevice(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDDevice());
}
void WebDSerialNum(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDSerialNum());
}
void WebDMaker(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDMaker());
}
void WebDCapacity(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDCapacity());
}
void WebDType(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDType());
}
void WebDTension(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDTension());
}
void WebDReportNum(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getDReportNum());
}
void WebDDivision(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDDivision());
}
void WebDZone(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDZone());
}
void WebDDate(int sock, PCSTR url)
{
	char buffer[12];
	getDateIntoBuffer(buffer, '-');
	fdprintf(sock, "%s", buffer);
	//fdprintf(sock, "%s", getDDate());
}
void WebDDateLastTest(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getDDateLastTest());
}
void WebTInitialState(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTInitialState());
}
void WebTCycleNumber(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTCycleNumber());
}
void WebTCycleWaitTime(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTCycleWaitTime());
}
void WebTCmdWaitTime(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTCmdWaitTime());
}
void WebTClosePulseTime(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTClosePulseTime());
}
void WebTOpenPulseTime(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTOpenPulseTime());
}
void WebTPhasesWaitTime(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTPhasesWaitTime());
}
void WebTMaxDiffClose(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTMaxDiffClose());
}
void WebTMaxDiffOpen(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTMaxDiffOpen());
}
void WebTMaxTimeAfterPulse(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTMaxTimeAfterPulse());
}
void WebAppVersion(int sock, PCSTR url)
{
	fdprintf(sock, "%s", APPVERSION);
}
void WebResults(int sock, PCSTR url)
{
	Fifo webData;
	getWebResults(&webData);
	if(webData.FifoSize() > 0){
		char * data = (char *)webData.Get();
		while(data){
			fdprintf(sock, "%s", data);
			webData.MoveReader(true);
			data = (char *)webData.Get();
		}
	}else{
		fdprintf(sock, "%s", "No data");
	}
}
void WebFasesStatus(int sock, PCSTR url)
{
	ProcessState state = getProcessState(true);
	fdprintf(sock, "%s", ProcessInfo[state]);
}

void WebFasesStatusAsyc(int sock, PCSTR url)
{
	ProcessState state = getProcessState(false);
	fdprintf(sock, "%s", ProcessInfo[state]);
}

void WebMicrometerStatus(int sock, PCSTR url)
{
	fdprintf(sock, "%s", ProcessInfo[getMicrometerState()]);
}

void WebDigitals(int sock, PCSTR url)
{
	struct InputsInfo *newInfo = getDigitals();
	for (int x = 0; x < MAX_DIG_INPUTS - CMDDIGITALSIN; ++x) {
		fdprintf(sock, "<label>%s: <span>%s</span></label><br>", newInfo[x].alias, newInfo[x].state > 0?"Cerrada":"Abierto");
	}
	free(newInfo);
	newInfo = NULL;
}

void WebMicrometerMode(int sock, PCSTR url)
{
	fdprintf(sock, "%s", ModeInfo[getMicrometerMode()]);
}

void WebMicrometerDateTime(int sock, PCSTR url)
{
	char bufferDate[12];
	getDateIntoBuffer(bufferDate, '-');
	char bufferTime[12];
	getTimeIntoBuffer(bufferTime, ':');
	fdprintf(sock, "%s %s", bufferDate, bufferTime);
}
/**/
