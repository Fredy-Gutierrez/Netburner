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
#include "libs/Default_Values.h"
#include "libs/Configurations.h"

/************************************************************GET FUNCTIONS FOR INPUTS CONFIG***************************************************************/
void WebConfigSys1(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSys1) );
}
void WebConfigSys1p(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSysP1) );
}
void WebConfigSys1n(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSysN1) );
}
void WebConfigSys2(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSys2) );
}
void WebConfigSys2p(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSysP2) );
}
void WebConfigSys2n(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSysN2) );
}
void WebConfigSys1np(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSysPN1) );
}
void WebConfigSys2np(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxSysPN2));
}
void WebConfigInC_I(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxLoopC_I) );
}
void WebConfigInO(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxLoopO) );
}
void WebConfigDcOk1(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxDcOk1) );
}
void WebConfigDcOk2(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxDcOk2) );
}
void WebConfigAcOk1(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxAcOk1) );
}
void WebConfigBat1(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxBat1) );
}
void WebConfigAcOk2(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxAcOk2) );
}
void WebConfigBat2(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxBat2) );
}
void WebConfigDoor(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxDoor) );
}
void WebConfigFree(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getInputByIndex(idxFree) );
}

/************************************************************GET FUNCTIONS FOR CONNECTIVITY CONFIG***************************************************************/
void WebConfigNbIpMode(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getNbIpMode());
}
void WebConfigNbIp(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getNbIpAddress());
}
void WebConfigNbMask(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getNbIpMask());
}
void WebConfigNbGate(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getNbIpGate());
}
void WebConfigNbDns(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getNbIpDns1());
}
void WebConfigSargasIp_R(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getSargasIp('R'));
}
void WebConfigSargasIp_L(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getSargasIp('L'));
}
void WebConfigQtySargasAntR(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getQtySargasAnt('R'));
}
void WebConfigQtySargasAntL(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getQtySargasAnt('L'));
}
void WebConfigUdpIp(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getUdpIp());
}
void WebConfigLogChoice(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getLogChoice());
}
void WebConfigUdpPort(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getUdpPort());
}
void WebConfigServerUrl(int sock, PCSTR url){
	fdprintf(sock, "%s", getServerUrl());
}

/************************************************************GET FUNCTIONS GENERAL CONFIG***************************************************************/
void WebConfigAeiId(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getAeiID());
}
void WebConfigLoopType(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getLoopType());
}
void WebConfigLoopTimeOn(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getLoopTimeOn());
}
void WebConfigLoopTimeOff(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getLoopTimeOff());
}
void WebConfigLoopOnOff(int sock, PCSTR url)
{
	if(getLoopRfidOnOff()){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigAeiDir(int sock, PCSTR url)
{
	if(getAeiDir() > 0){
		fdprintf(sock, "%d", 1);
	}else{
		fdprintf(sock, "%d", 0);
	}

}
void WebConfigNtpSyncOnOff(int sock, PCSTR url)
{
	if(getNtpSyncOnOff()){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigNtpServer(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getNtpServer());
}
void WebConfigNtpSyncTime(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getNtpSyncTime());
}
void WebConfigPhoneNumber(int sock, PCSTR url)
{
	fdprintf(sock, "%s", getPhoneNumber());
}
void WebConfigBattTestOnOff(int sock, PCSTR url)
{
	if(getBattTestOnOff()){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigBattInstallDate(int sock, PCSTR url)
{
	fdprintf(sock, "%s", convertSecToAsciiDate(getBattInstallDate()));
}
void WebConfigBattTestPeriod(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getBattTestPeriod());
}
void WebConfigTempAlarmSet(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getTempAlarmSet());
}
void WebConfigBattTestHour(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getBattTestHour());
}
void WebConfigMainADSignal(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getMainADSignal());
}
void WebConfigDoubleAxDet(int sock, PCSTR url)
{
	if(getDoubleAxDet()){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigDoubleADSignal(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getDoubleADSignal());
}
void WebConfigReverseDoubleAD(int sock, PCSTR url)
{
	if(getDirDoubleAxDet()==-1){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigMirrorAxDet(int sock, PCSTR url)
{
	if(getMirrorAxDet()){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigMirrorADSignal(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getMirrorADSignal());
}
void WebConfigReverseMirrorAD(int sock, PCSTR url)
{
	if(getDirMirrorAxDet()==-1){
		fdprintf(sock, "%s", "checked");
	}else{
		fdprintf(sock, "%s", "");
	}
}
void WebConfigQtySargas(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getQtySargas());
}
/***********************************************FUNCTIONS FOR ADDITIONALS INFORMATIONS*************************************************/
void WebConfigAeiStatus(int sock, PCSTR url)
{
	fdprintf(sock, "%d", getAeiStatus());
}
void WebConfigAppVersion(int sock, PCSTR url)
{
	fdprintf(sock, "%s", APPVERSION);
}
void WebConfigReports(int sock, PCSTR url)
{
	for(int x = 1; x<11; x++){
		fdprintf(sock, "<tr> <th scope=\"row\">%d</th> <td>Status%d</td> <td>%d</td> </tr>",x,x,x);
	}

}
