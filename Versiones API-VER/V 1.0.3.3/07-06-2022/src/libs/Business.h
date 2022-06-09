/*
 * Business.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef BUSINESS_H_
#define BUSINESS_H_

#include "libs/Rtc_Controller.h"
#include "libs/Log.h"

enum typeService { infoStartCarDet, infoEndCarDet, infoCarStopOver, repDigChg, repMainAxle, repSecAxle, repMirrorAxle, infoGeneral, turnOnAnt, turnOffAnt, noService };

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	void printLogs(void);
	bool getSychStatus();
	void initBusiness();
	void attendBusiness();
	void askForService(uint32_t format, void * obj, bool saveCmd);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


class speed {
public:
	uint32_t value;
	int direction;

	speed();
	uint32_t getValue();
	int getDir();
	void updateSpeed(uint32_t _speed, int _direction);
	void reset();
};

class clsBattery {
public:
	int state;
	bool testOn;
	int tick;
	bool replace;
	bool testByUserRequest;

	clsBattery();
	void timer();
	void test();
	bool getTestOn();
	void updateIo();
};


typedef struct{
	char id[21];
	char status;
	int code;
	typeAeiTime t;
}stData;

#define MAX_STATUS_PAR		8
enum stIndex {idxMac30, idxTagReader, idxAxleDet, idxLoopDet, idxDoorSt, idxAcPower, idxDcPower, idxBattery};

class clsStatus {
public:
	stData par[MAX_STATUS_PAR];

	clsStatus();
	const char * getStatus();
	void setStatus(int index, char _status, int _code);
	char getStatusIdx(int index);
	void updateStatus();
};

extern clsStatus stMac30;
extern logger logg;
extern speed speedDmh;

extern bool testMode;

void initSargas();

#endif /* BUSINESS_H_ */
