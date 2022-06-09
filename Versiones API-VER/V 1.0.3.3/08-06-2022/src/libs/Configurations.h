/*
 * Configurations.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_CONFIGURATIONS_H_
#define LIBS_CONFIGURATIONS_H_
//<#includes>

#include <basictypes.h>
#include <netinterface.h>
#include "libs/IO_Interface.h"

#define MAX_USER_PARAM_SIZE 8192//8K
#define VERIFY_KEY 0x48666055  // NV Settings key code


//</#includes>
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

/************************************************
 * GETTERS AREA*
 * **********************************************/

	/**************************************************GETTERS FOR CONNECTIVITIES CONFIGURATIONS*************************************/
	const char * getNbIpMode();
	const char * getNbIpAddress();
	const char * getNbIpMask();
	const char * getNbIpGate();
	const char * getNbIpDns1();

	int getQtySargas();
	const char* getQtySargasStr();
	const char * getSargasIp(char id);
	int getQtySargasAnt(char id);

	int getLogChoice();
	const char * getLogChoiceStr();
	int getUdpPort();
	const char * getUdpIp();

	const char * getServerUrl();

	/**************************************************GETTERS FOR GPIO CONFIGURATIONS*************************************/
	int getInputByIndex(idxIoSignals index);

	/**************************************************GETTERS FOR GENERAL CONFIGURATIONS*************************************/
	const char * getAeiID();
	int getAeiDir();
	const char * getAeiDirStr();
	int getLoopType();
	const char * getLoopTypeStr();
	int getLoopTimeOn();
	int getLoopTimeOff();
	bool getLoopRfidOnOff();
	bool getNtpSyncOnOff();
	const char * getNtpServer();
	int getNtpSyncTime();
	bool getBattTestOnOff();
	long long getBattInstallDate();
	int getBattTestPeriod();
	int getBattTestHour();
	long long getBattTestLast();

	int getMainADSignal();
	idxIoSignals getMainADSignalIndex();
	bool getDoubleAxDet();
	int getDoubleADSignal();
	int getDirDoubleAxDet();
	idxIoSignals getDoubleADSignalIndex();
	bool getMirrorAxDet();
	int getMirrorADSignal();
	int getDirMirrorAxDet();
	idxIoSignals getMirrorADSignalIndex();

	int getTempAlarmSet();
	const char * getPhoneNumber();

	/**************************************************GETTERS FOR ADDITIONAL CONFIGURATIONS*************************************/
	int getCarCounter();
	int getTrainCounter();
	int getStatusReportingTime();
	int getAeiStatus();

/************************************************
 * SETTERS AREA*
 * **********************************************/

	/********************************************SETTERS FOR CONNECTIVITIES CONFIGURATIONS***********************************************/
	void setNbIpMode(char * value);
	void setNbIpAddress(char * value);
	void setNbIpMask(char * value);
	void setNbIpGate(char * value);
	void setNbIpDns1(char * value);

	void setLogChoice(int val);
	void setUdpIp(char * newUdpIp);
	void setUdpPort(int port);

	void setSargasQty(int val);
	void setSargasIpR(char * newIp);
	void setSargasAntQtyR(int val);
	void setSargasIpL(char * newIp);
	void setSargasAntQtyL(int val);

	void setServerUrl(char * newUrl);

	/********************************************SETTERS FOR GPIO CONFIGURATIONS***********************************************/
	void setInSys1(int newVal);
	void setInSys1p(int newVal);
	void setInSys1n(int newVal);
	void setInSys2(int newVal);
	void setInSys2p(int newVal);
	void setInSys1np(int newVal);
	void setInSys2n(int newVal);
	void setInSys2np(int newVal);
	void setInLoopC_I(int newVal);
	void setInLoopO(int newVal);
	void setInDcOk1(int newVal);
	void setInDcOk2(int newVal);
	void setInAcOk1(int newVal);
	void setInBat1(int newVal);
	void setInAcOk2(int newVal);
	void setInBat2(int newVal);
	void setInDoor(int newVal);
	void setInFree(int newVal);

	/********************************************SETTERS FOR GENERAL CONFIGURATIONS***********************************************/
	void setAeiID(char * value);
	void setAeiDir(bool val);
	void setLoopType(int type);
	void setLoopTimeOn(int time);
	void setLoopTimeOff(int time);
	void setLoopRfidOnOff(bool val);
	void setNtpSyncOnOff(bool val);
	void setNtpServer(char * value);
	void setNtpSyncTime(int value);
	void setBattTestOnOff(bool val);
	void setBattInstallDate(long long value);
	void setBattTestPeriod(int value);
	void setBattTestHour(int value);
	void setBattTestLast(long long value);

	void setMainADSignal(int value);
	void setDoubleAxDet(bool value);
	void setDoubleADSignal(int value);
	void setDirDoubleAxDet(int value);
	void setMirrorAxDet(bool value);
	void setMirrorADSignal(int value);
	void setDirMirrorAxDet(int value);

	void setTempAlarmSet(int value);
	void setPhoneNumber(char * value);
	/********************************************SETTERS FOR ADDITIONAL CONFIGURATIONS***********************************************/
	void setStatusReportingTime(int value);
	void setAeiStatus(int value);
	void incrementCarCounter();
	void incrementTrainCounter();
	/********************************************FUNCTIONS FOR UTILITIES CONFIGURATIONS***********************************************/
	void saveSettings();
	void saveConfigToStorage();
	void restartMicroChip();
	void changeToStaticMode();
	void changeToDHCPMode();
	void restartDHCP();
	void checkEth0Config();
	void getConfigurations();
	void restoreDefault();
	long long convertDateToSec(const char * date);
	char * convertSecToAsciiDate(long long dateInSec);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_CONFIGURATIONS_H_ */
