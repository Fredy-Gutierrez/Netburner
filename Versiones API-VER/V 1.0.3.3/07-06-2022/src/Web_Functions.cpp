/*
 * Web_Functions.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
//2021-05-15
#include <http.h>
#include <iosys.h>
#include <httppost.h>
#include <fdprintf.h>
#include <config_obj.h>
#include <string.h>
#include "libs/Rtc_Controller.h"
#include "libs/Configurations.h"
#include "libs/SargasProcess.h"
#include "libs/Web_Functions.h"

/************************************************STRUCTS FOR OLD AND NEW CONFIGURATIONS*********************************************/
struct SargasConfig oldSargasConf;
struct SargasConfig newSargasConf;

struct UdpConfig oldUdpConf;
struct UdpConfig newUdpConf;

struct ServerConfig oldServerConf;
struct ServerConfig newServerConf;

struct inputStruct oldInConf;
struct inputStruct newInConf;

struct EthernetConfig oldEthernetConf;
struct EthernetConfig newEthernetConf;

struct GeneralConfig oldGeneralConf;
struct GeneralConfig newGeneralConf;

static const struct GeneralConfig cleanGeneralConf{"",0,0,0,false,false,false,"",0,"",false,"",0,0,0,0,false,0,false,0,false,false};
static const struct SargasConfig cleanSargasConfig{0,"","",0,0};
/************************************************VALIDATIONS FUNCTIONS*********************************************/
void validateSargasValues(){
	if(oldSargasConf.SargasQty != newSargasConf.SargasQty){
		setSargasQty(newSargasConf.SargasQty);  }
	if(strcmp(oldSargasConf.ip_r.c_str(), newSargasConf.ip_r.c_str()) != 0 && strcmp(newSargasConf.ip_r.c_str(), "") != 0){
		setSargasIpR(strdup(newSargasConf.ip_r.c_str())); }
	if(strcmp(oldSargasConf.ip_l.c_str(), newSargasConf.ip_l.c_str()) != 0 && strcmp(newSargasConf.ip_l.c_str(), "") != 0){
		setSargasIpL(strdup(newSargasConf.ip_l.c_str())); }
	if(oldSargasConf.antQtyR != newSargasConf.antQtyR){
		setSargasAntQtyR(newSargasConf.antQtyR);  }
	if(oldSargasConf.antQtyL != newSargasConf.antQtyL && newSargasConf.antQtyL != 0){
		setSargasAntQtyL(newSargasConf.antQtyL);  }
	saveSettings();
	newSargasConf = cleanSargasConfig;
}
void validateUdpValues(){
	if(strcmp(oldUdpConf.ip.c_str(), newUdpConf.ip.c_str()) != 0 && strcmp(newUdpConf.ip.c_str(), "") != 0){
		setUdpIp(strdup(newUdpConf.ip.c_str())); }
	if(oldUdpConf.port != newUdpConf.port && newUdpConf.port != 0){
		setUdpPort(newUdpConf.port);  }
	if(oldUdpConf.LogChoice != newUdpConf.LogChoice){
		setLogChoice(newUdpConf.LogChoice);  }
	saveSettings();
}
void validateServerValues(){
	if(strcmp(oldServerConf.url, newServerConf.url) != 0 && strcmp(newServerConf.url, "") != 0){
		setServerUrl(newServerConf.url); }
	saveSettings();
}
void validateInValues(){
	if(oldInConf.Sys1 != newInConf.Sys1 && newInConf.Sys1 != 0){ setInSys1(newInConf.Sys1); }
	if(oldInConf.Sys1p != newInConf.Sys1p && newInConf.Sys1p != 0){ setInSys1p(newInConf.Sys1p); }

	if(oldInConf.Sys1n != newInConf.Sys1n && newInConf.Sys1n != 0){ setInSys1n(newInConf.Sys1n); }

	if(oldInConf.Sys2 != newInConf.Sys2 && newInConf.Sys2 != 0){ setInSys2(newInConf.Sys2); }
	if(oldInConf.Sys2p != newInConf.Sys2p && newInConf.Sys2p != 0){ setInSys2p(newInConf.Sys2p); }

	if(oldInConf.Sys1np != newInConf.Sys1np && newInConf.Sys1np != 0){ setInSys1np(newInConf.Sys1np); }
	if(oldInConf.Sys2n != newInConf.Sys2n && newInConf.Sys2n != 0){ setInSys2n(newInConf.Sys2n); }
	if(oldInConf.Sys2np != newInConf.Sys2np && newInConf.Sys2np != 0){ setInSys2np(newInConf.Sys2np); }

	if(oldInConf.LoopC_I != newInConf.LoopC_I && newInConf.LoopC_I != 0){ setInLoopC_I(newInConf.LoopC_I); }
	if(oldInConf.LoopO != newInConf.LoopO && newInConf.LoopO != 0){ setInLoopO(newInConf.LoopO); }
	if(oldInConf.DcOk1 != newInConf.DcOk1 && newInConf.DcOk1 != 0){ setInDcOk1(newInConf.DcOk1); }
	if(oldInConf.DcOk2 != newInConf.DcOk2 && newInConf.DcOk2 != 0){ setInDcOk2(newInConf.DcOk2); }
	if(oldInConf.AcOk1 != newInConf.AcOk1 && newInConf.AcOk1 != 0){ setInAcOk1(newInConf.AcOk1); }
	if(oldInConf.Bat1 != newInConf.Bat1 && newInConf.Bat1 != 0){ setInBat1(newInConf.Bat1); }
	if(oldInConf.AcOk2 != newInConf.AcOk2 && newInConf.AcOk2 != 0){ setInAcOk2(newInConf.AcOk2); }
	if(oldInConf.Bat2 != newInConf.Bat2 && newInConf.Bat2 != 0){ setInBat2(newInConf.Bat2); }
	if(oldInConf.Door != newInConf.Door && newInConf.Door != 0){ setInDoor(newInConf.Door); }
	if(oldInConf.Free != newInConf.Free && newInConf.Free != 0){ setInFree(newInConf.Free); }
	saveSettings();
}
void validateEthernetValues(){
	bool ipchange = false;
	if(strcmp(oldEthernetConf.mode.c_str(), newEthernetConf.mode.c_str()) != 0 ){
		setNbIpMode(strdup(newEthernetConf.mode.c_str()));
		if(strcmp(newEthernetConf.mode.c_str(), "DHCP") == 0){
			changeToDHCPMode();
		}else{
			setNbIpAddress(strdup(newEthernetConf.ip.c_str()));
			setNbIpMask(strdup(newEthernetConf.mask.c_str()));
			setNbIpGate(strdup(newEthernetConf.gate.c_str()));
			setNbIpDns1(strdup(newEthernetConf.dns1.c_str()));
			changeToStaticMode();
		}
		saveSettings();
		saveConfigToStorage();
		restartMicroChip();
	}else{
		if(strcmp(oldEthernetConf.mode.c_str(), "DHCP") != 0){
			//bool change = false;
			if(strcmp(oldEthernetConf.ip.c_str(), newEthernetConf.ip.c_str()) != 0 && strcmp(newEthernetConf.ip.c_str(), "") != 0){
				setNbIpAddress(strdup(newEthernetConf.ip.c_str())); ipchange = true;
			}

			if(strcmp(oldEthernetConf.mask.c_str(), newEthernetConf.mask.c_str()) != 0 && strcmp(newEthernetConf.mask.c_str(), "") != 0){
				setNbIpMask(strdup(newEthernetConf.mask.c_str())); ipchange = true;
			}else if(ipchange){
				setNbIpMask(strdup(newEthernetConf.mask.c_str()));
			}

			if(strcmp(oldEthernetConf.gate.c_str(), newEthernetConf.gate.c_str()) != 0 && strcmp(newEthernetConf.gate.c_str(), "") != 0){
				setNbIpGate(strdup(newEthernetConf.gate.c_str())); ipchange = true;
			}else if(ipchange){
				setNbIpGate(strdup(newEthernetConf.gate.c_str()));
			}

			if(strcmp(oldEthernetConf.dns1.c_str(), newEthernetConf.dns1.c_str()) != 0 && strcmp(newEthernetConf.dns1.c_str(), "") != 0){
				setNbIpDns1(strdup(newEthernetConf.dns1.c_str())); ipchange = true;
			}else if(ipchange){
				setNbIpDns1(strdup(newEthernetConf.dns1.c_str()));
			}
			saveSettings();
			if(ipchange){
				saveConfigToStorage();
				changeToStaticMode();
				restartMicroChip();
			}
		}
	}
}
void validateGeneralValues(){
	if(strcmp(oldGeneralConf.AeiID.c_str(), newGeneralConf.AeiID.c_str()) != 0 && strcmp(newGeneralConf.AeiID.c_str(), "") != 0){
		setAeiID(strdup(newGeneralConf.AeiID.c_str()));  }
	if(oldGeneralConf.LoopType != newGeneralConf.LoopType){
		setLoopType(newGeneralConf.LoopType);  }
	if(oldGeneralConf.LoopTimeOn != newGeneralConf.LoopTimeOn && newGeneralConf.LoopTimeOn != 0){
		setLoopTimeOn(newGeneralConf.LoopTimeOn);  }
	if(oldGeneralConf.LoopTimeOff != newGeneralConf.LoopTimeOff && newGeneralConf.LoopTimeOff != 0){
		setLoopTimeOff(newGeneralConf.LoopTimeOff);  }
	if(int(oldGeneralConf.LoopRfidOnOff) != int(newGeneralConf.LoopRfidOnOff)){
		setLoopRfidOnOff(newGeneralConf.LoopRfidOnOff);  }
	if(int(oldGeneralConf.AeiDir) != int(newGeneralConf.AeiDir)){
		setAeiDir(newGeneralConf.AeiDir); }
	if(int(oldGeneralConf.NtpSyncOnOff) != int(newGeneralConf.NtpSyncOnOff)){
		setNtpSyncOnOff(newGeneralConf.NtpSyncOnOff);  }
	if(strcmp(oldGeneralConf.NtpServer.c_str(), newGeneralConf.NtpServer.c_str()) != 0 && strcmp(newGeneralConf.NtpServer.c_str(), "") != 0){
		setNtpServer(strdup(newGeneralConf.NtpServer.c_str()));  }
	if(oldGeneralConf.NtpSyncTime != newGeneralConf.NtpSyncTime && newGeneralConf.NtpSyncTime != 0){
		setNtpSyncTime(newGeneralConf.NtpSyncTime); }
	if(strcmp(oldGeneralConf.PhoneNumber.c_str(), newGeneralConf.PhoneNumber.c_str()) != 0 && strcmp(newGeneralConf.PhoneNumber.c_str(), "") != 0){
		setPhoneNumber(strdup(newGeneralConf.PhoneNumber.c_str())); }
	if(int(oldGeneralConf.BattTestOnOff) != int(newGeneralConf.BattTestOnOff)){
		setBattTestOnOff(newGeneralConf.BattTestOnOff);  }
	if(strcmp(oldGeneralConf.BattInstallDate.c_str(), newGeneralConf.BattInstallDate.c_str()) != 0){
		setBattInstallDate(convertDateToSec(newGeneralConf.BattInstallDate.c_str())); }
	if(oldGeneralConf.BattTestPeriod != newGeneralConf.BattTestPeriod && newGeneralConf.BattTestPeriod != 0){
		setBattTestPeriod(newGeneralConf.BattTestPeriod); }
	if(oldGeneralConf.TempAlarmSet != newGeneralConf.TempAlarmSet && newGeneralConf.TempAlarmSet != 0){
		setTempAlarmSet(newGeneralConf.TempAlarmSet); }
	if(oldGeneralConf.BattTestHour != newGeneralConf.BattTestHour && newGeneralConf.BattTestHour != 0){
		setBattTestHour(newGeneralConf.BattTestHour); }
	if(oldGeneralConf.MainADSignal != newGeneralConf.MainADSignal){
		setMainADSignal(newGeneralConf.MainADSignal); }
	if(int(oldGeneralConf.DoubleAxlesDetector) != int(newGeneralConf.DoubleAxlesDetector)){
		setDoubleAxDet(newGeneralConf.DoubleAxlesDetector);
		if(oldGeneralConf.DoubleADSignal != newGeneralConf.DoubleADSignal){
				setDoubleADSignal(newGeneralConf.DoubleADSignal); }
	}

	if(int(oldGeneralConf.MirrorAxlesDetector) != int(newGeneralConf.MirrorAxlesDetector)){
		setMirrorAxDet(newGeneralConf.MirrorAxlesDetector);

		if(oldGeneralConf.MirrorADSignal != newGeneralConf.MirrorADSignal){
			setMirrorADSignal(newGeneralConf.MirrorADSignal); }
	}

	if(int(oldGeneralConf.ReverseDoubleAD) != int(newGeneralConf.ReverseDoubleAD)){
		if(!newGeneralConf.ReverseDoubleAD){
			setDirDoubleAxDet(1);
		}else {
			setDirDoubleAxDet(-1);
		}
	}
	if(int(oldGeneralConf.ReverseMirrorAD) != int(newGeneralConf.ReverseMirrorAD)){
		if(!newGeneralConf.ReverseMirrorAD)	{
			setDirMirrorAxDet(1);
		}else {
			setDirMirrorAxDet(-1);
		}
	}

	saveSettings();
	newGeneralConf = cleanGeneralConf;
}

/************************************************GETS THE OLD DATA*********************************************/
void getOldSargasValues(){
	oldSargasConf.SargasQty = getQtySargas();
	oldSargasConf.ip_r = (NBString)getSargasIp('R');
	oldSargasConf.ip_l = (NBString)getSargasIp('L');
	oldSargasConf.antQtyR = getQtySargasAnt('R');
	oldSargasConf.antQtyL = getQtySargasAnt('L');
}
void getOldUdpValues(){
	oldUdpConf.LogChoice = getLogChoice();
	oldUdpConf.ip = (NBString)getUdpIp();
	oldUdpConf.port = getUdpPort();
}
void getOldServerValues(){
	//oldServerConf.url = getServerUrl();
	char * oldUrl = (char*)getServerUrl();
	memcpy(oldServerConf.url, oldUrl, strlen(oldUrl));
}
void getOldInValues(){
	oldInConf.Sys1 = getInputByIndex(idxSys1);
	oldInConf.Sys1p = getInputByIndex(idxSysP1);
	oldInConf.Sys1n = getInputByIndex(idxSysN1);
	oldInConf.Sys2 = getInputByIndex(idxSys2);
	oldInConf.Sys2p = getInputByIndex(idxSysP2);
	oldInConf.Sys2n = getInputByIndex(idxSysN2);
	oldInConf.Sys1np = getInputByIndex(idxSysPN1);
	oldInConf.Sys2np = getInputByIndex(idxSysPN2);
	oldInConf.LoopC_I = getInputByIndex(idxLoopC_I);
	oldInConf.LoopO = getInputByIndex(idxLoopO);
	oldInConf.DcOk1 = getInputByIndex(idxDcOk1);
	oldInConf.DcOk2 = getInputByIndex(idxDcOk2);
	oldInConf.AcOk1 = getInputByIndex(idxAcOk1);
	oldInConf.AcOk2 = getInputByIndex(idxAcOk2);
	oldInConf.Bat1 = getInputByIndex(idxBat1);
	oldInConf.Bat2 = getInputByIndex(idxBat2);
	oldInConf.Door = getInputByIndex(idxDoor);
	oldInConf.Free = getInputByIndex(idxFree);
}
void getOldEthernetValues(){
	oldEthernetConf.mode = (NBString)getNbIpMode();
	oldEthernetConf.ip = (NBString)getNbIpAddress();
	oldEthernetConf.mask = (NBString)getNbIpMask();
	oldEthernetConf.gate = (NBString)getNbIpGate();
	oldEthernetConf.dns1 = (NBString)getNbIpDns1();
}
void getOldGeneralValues(){
	oldGeneralConf.AeiID = (NBString)getAeiID();
	oldGeneralConf.LoopType = getLoopType();
	oldGeneralConf.LoopTimeOn = getLoopTimeOn();
	oldGeneralConf.LoopTimeOff = getLoopTimeOff();
	oldGeneralConf.LoopRfidOnOff = getLoopRfidOnOff();
	if(getAeiDir() > 0){
		oldGeneralConf.AeiDir = true;
	}else{
		oldGeneralConf.AeiDir = false;
	}
	oldGeneralConf.NtpSyncOnOff = getNtpSyncOnOff();
	oldGeneralConf.NtpServer = (NBString)getNtpServer();
	oldGeneralConf.NtpSyncTime = getNtpSyncTime();
	oldGeneralConf.PhoneNumber = (NBString)getPhoneNumber();
	oldGeneralConf.BattTestOnOff = getBattTestOnOff();
	oldGeneralConf.BattInstallDate = convertSecToAsciiDate(getBattInstallDate());
	oldGeneralConf.BattTestPeriod = getBattTestPeriod();
	oldGeneralConf.TempAlarmSet = getTempAlarmSet();
	oldGeneralConf.BattTestHour = getBattTestHour();
	oldGeneralConf.MainADSignal = getMainADSignal();
	oldGeneralConf.DoubleAxlesDetector = getDoubleAxDet();
	oldGeneralConf.DoubleADSignal = getDoubleADSignal();
	oldGeneralConf.MirrorAxlesDetector = getMirrorAxDet();
	oldGeneralConf.MirrorADSignal = getMirrorADSignal();

	if(getDirDoubleAxDet()==1)
		oldGeneralConf.ReverseDoubleAD = false;
	else
		oldGeneralConf.ReverseDoubleAD = true;
	if(getDirMirrorAxDet()==1)
		oldGeneralConf.ReverseMirrorAD = false;
	else
		oldGeneralConf.ReverseMirrorAD = true;

	newGeneralConf.MirrorADSignal = getMirrorADSignal();
	//memcpy(&newGeneralConf, &oldGeneralConf, sizeof(GeneralConfig));
}

/*********************************************FUNCTIONS THAT PROCESS THE DATA RECEIVED FROM SARGASFORM***********************************************/
void FormPostSargas(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "puerto") == 0) {
					//comm.UpdateConfigPort(pValue);
			} else if (strcmp(pName, "ip_r") == 0) {
				newSargasConf.ip_r = (NBString)pValue;
			} else if (strcmp(pName, "ip_l") == 0) {
				newSargasConf.ip_l = (NBString)pValue;
			} else if (strcmp(pName, "sargasAntQtyR") == 0) {
				newSargasConf.antQtyR = atoi(pValue);
			} else if (strcmp(pName, "sargasAntQtyL") == 0) {
				newSargasConf.antQtyL = atoi(pValue);
			}else if (strcmp(pName, "SargasQty") == 0) {
				newSargasConf.SargasQty = atoi(pValue);
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
				//writestring(sock, "{status: 200}");
				validateSargasValues();
				RedirectResponse(sock, "server_config.html");
			}
			break;
	}
}

/*********************************************FUNCTION THAT RECEIVES THE WEB SERVER DATA FROM SERVERFORM***********************************************/
void FormPostServer(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "server_address") == 0) {
				//newServerConf.url = (NBString)pValue;
				memcpy(newServerConf.url, pValue, strlen(pValue));
				newServerConf.url[strlen(pValue)] = '\0';
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
				//writestring(sock, "{status: 200}");
				validateServerValues();
				RedirectResponse(sock, "server_config.html");
			}
			break;
	}
}

/*********************************************FUNCTION THAT RECEIVES UDP DATA FROM UDPFORM***********************************************/
void FormPostUdp(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "udpPort") == 0) {
				newUdpConf.port = atoi(pValue);
			} else if (strcmp(pName, "udpIp") == 0) {
				newUdpConf.ip = (NBString)pValue;
			}else if (strcmp(pName, "LogChoice") == 0) {
				newUdpConf.LogChoice = atoi(pValue);
			}
			//LogChoice
			break;
		case eFile:
			break;
		case eEndOfPost: {
			validateUdpValues();
			RedirectResponse(sock, "server_config.html");
			//writestring(sock, "{status: 200}");
		}
		break;
	}
}

/*********************************************FUNCTION THAT RECEIVES IO DATA FROM IOFORM***********************************************/
void FormPostIO(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "Sys1") == 0) {
				newInConf.Sys1 = atoi(pValue);
			} else if (strcmp(pName, "Sys1p") == 0) {
				newInConf.Sys1p = atoi(pValue);
			} else if (strcmp(pName, "Sys1n") == 0) {
				newInConf.Sys1n = atoi(pValue);
			}else if (strcmp(pName, "Sys2") == 0) {
				newInConf.Sys2 = atoi(pValue);
			}else if (strcmp(pName, "Sys2p") == 0) {
				newInConf.Sys2p = atoi(pValue);
			}else if (strcmp(pName, "Sys1np") == 0) {
				newInConf.Sys1np = atoi(pValue);
			}else if (strcmp(pName, "Sys2n") == 0) {
				newInConf.Sys2n = atoi(pValue);
			}else if (strcmp(pName, "Sys2np") == 0) {
				newInConf.Sys2np = atoi(pValue);
			}else if (strcmp(pName, "LoopC_I") == 0) {
				newInConf.LoopC_I = atoi(pValue);
			}else if (strcmp(pName, "LoopO") == 0) {
				newInConf.LoopO = atoi(pValue);
			}else if (strcmp(pName, "DcOk1") == 0) {
				newInConf.DcOk1 = atoi(pValue);
			}else if (strcmp(pName, "DcOk2") == 0) {
				newInConf.DcOk2 = atoi(pValue);
			}else if (strcmp(pName, "AcOk1") == 0) {
				newInConf.AcOk1 = atoi(pValue);
			}else if (strcmp(pName, "Bat1") == 0) {
				newInConf.Bat1 = atoi(pValue);
			}else if (strcmp(pName, "AcOk2") == 0) {
				newInConf.AcOk2 = atoi(pValue);
			}else if (strcmp(pName, "Bat2") == 0) {
				newInConf.Bat2 = atoi(pValue);
			}else if (strcmp(pName, "Door") == 0) {
				newInConf.Door = atoi(pValue);
			}else if (strcmp(pName, "Free") == 0) {
				newInConf.Free = atoi(pValue);
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
			validateInValues();
			RedirectResponse(sock, "gpio.html");
			//writestring(sock, "{status: 200}");
		}
		break;
	}
}

/*********************************************FUNCTION THAT RECEIVES ETHERNET DATA FROM ETHERNETFORM***********************************************/
void FormPostEthernet(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "netburner_ip") == 0) {
				newEthernetConf.ip = (NBString)pValue;
			} else if (strcmp(pName, "netburner_mask") == 0) {
				newEthernetConf.mask = (NBString)pValue;
			} else if (strcmp(pName, "netburner_gate") == 0) {
				newEthernetConf.gate = (NBString)pValue;
			}else if (strcmp(pName, "netburner_dns") == 0) {
				newEthernetConf.dns1 = (NBString)pValue;
			}else if (strcmp(pName, "IpMode") == 0) {
				newEthernetConf.mode = (NBString)pValue;
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
			validateEthernetValues();
			RedirectResponse(sock, "server_config.html");
		}
		break;
	}
}

/*********************************************FUNCTION THAT RECEIVES ETHERNET DATA FROM ETHERNETFORM***********************************************/
void FormPostGeneral(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "AeiID") == 0) {
				newGeneralConf.AeiID = (NBString)pValue;
			} else if (strcmp(pName, "LoopType") == 0) {
				newGeneralConf.LoopType = atoi(pValue);
			} else if (strcmp(pName, "LoopTimeOn") == 0) {
				newGeneralConf.LoopTimeOn = atoi(pValue);
			}else if (strcmp(pName, "LoopTimeOff") == 0) {
				newGeneralConf.LoopTimeOff = atoi(pValue);
			}else if (strcmp(pName, "LoopOnOff") == 0) {
				newGeneralConf.LoopRfidOnOff = true;
			}else if (strcmp(pName, "AeiDir") == 0) {
				newGeneralConf.AeiDir = atoi(pValue);
			}else if (strcmp(pName, "NtpSyncOnOff") == 0) {
				newGeneralConf.NtpSyncOnOff = true;
			}else if (strcmp(pName, "NtpServer") == 0) {
				newGeneralConf.NtpServer = (NBString)pValue;
			}else if (strcmp(pName, "SyncTime") == 0) {
				newGeneralConf.NtpSyncTime = atoi(pValue);
			}else if (strcmp(pName, "PhoneNumber") == 0) {
				newGeneralConf.PhoneNumber = (NBString)pValue;
			}else if (strcmp(pName, "BattTestOnOff") == 0) {
				newGeneralConf.BattTestOnOff = true;
			}else if (strcmp(pName, "BateryInstallationDate") == 0) {
				newGeneralConf.BattInstallDate = (NBString)pValue;
			}else if (strcmp(pName, "BateryCycle") == 0) {
				newGeneralConf.BattTestPeriod = atoi(pValue);
			}else if (strcmp(pName, "TempAlarmSet") == 0) {
				newGeneralConf.TempAlarmSet = atoi(pValue);
			}else if (strcmp(pName, "BateryTestTime") == 0) {
				newGeneralConf.BattTestHour = atoi(pValue);
			}else if (strcmp(pName, "MainADSignal") == 0) {
				newGeneralConf.MainADSignal = atoi(pValue);
			}else if (strcmp(pName, "DoubleAxDet") == 0) {
				newGeneralConf.DoubleAxlesDetector = true;
			}else if (strcmp(pName, "DoubleADSignal") == 0) {
				newGeneralConf.DoubleADSignal = atoi(pValue);
			}else if (strcmp(pName, "MirrorAxDet") == 0) {
				newGeneralConf.MirrorAxlesDetector = true;
			}else if (strcmp(pName, "MirrorADSignal") == 0) {
				newGeneralConf.MirrorADSignal = atoi(pValue);
			}else if (strcmp(pName, "RDirDoubleAxDet") == 0) {
				newGeneralConf.ReverseDoubleAD = true;
			}else if (strcmp(pName, "RDirMirrorAxDet") == 0) {
				newGeneralConf.ReverseMirrorAD = true;
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
			validateGeneralValues();
			RedirectResponse(sock, "general_config.html");
		}
		break;
	}
}

NBString action = "";
NBString form = "";
void FormPostOpenClose(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			action = "";
			form = "";
			break;
		case eVariable:
			if (strcmp(pName, "action") == 0) {
				action = (NBString)pValue;
			}else if (strcmp(pName, "form") == 0) {
				form = (NBString)pValue;
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
			if (strcmp(action.c_str(), "open") == 0){
				if (strcmp(form.c_str(), "formSargas") == 0){
					getOldSargasValues();
				}else if (strcmp(form.c_str(), "formUdpConfig") == 0) {
					getOldUdpValues();
				}else if (strcmp(form.c_str(), "formServerConfig") == 0) {
					getOldServerValues();
				}else if (strcmp(form.c_str(), "formIO") == 0) {
					getOldInValues();
				}else if (strcmp(form.c_str(), "formNetburner") == 0) {
					getOldEthernetValues();
				}else if (strcmp(form.c_str(), "formGeneral") == 0) {
					getOldGeneralValues();
				}
			}
			writestring(sock, "{status: 200}");
		}
		break;
	}
}


//formIO
//Register a call back for the web page...
HtmlPostVariableListCallback filePostForm("formSargas*", FormPostSargas);
//formServerConfig
HtmlPostVariableListCallback filePostFormServer("formServerConfig*", FormPostServer);

HtmlPostVariableListCallback filePostFormUdp("formUdpConfig*", FormPostUdp);

HtmlPostVariableListCallback filePostOpenClose("open_close*", FormPostOpenClose);

HtmlPostVariableListCallback filePostFormIO("formIO*", FormPostIO);

HtmlPostVariableListCallback filePostFormEthernet("formNetburner*", FormPostEthernet);

HtmlPostVariableListCallback filePostFormGeneral("formGeneral*", FormPostGeneral);





