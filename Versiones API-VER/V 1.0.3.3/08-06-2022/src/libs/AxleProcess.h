/*
 * AxleProcess.h
 *
 *  Created on: 25 april 2021
 *      Author: Integra MGZ
*/

#ifndef AXLEPROCESS_H_
#define AXLEPROCESS_H_

#include "libs/Rtc_Controller.h"
#include "libs/fifo/Fifo.h"
#include "libs/TagUtilities.h"

typedef struct{
	typeAeiTime time;
	int speed;
	int dir;
	int distance;
	int carNumber;
	bool tagged;
}typeAxle;

#define MAX_AXLES_TRAIN		2000
#define MAX_CARS_TRAIN		200
#define MAX_SEGS_TRAIN		20

typedef struct{
	bool processedFlag;
	char packetId[12];
	int qtyAxles;
	int qtyHexTags;
	typeHexTag hexTagData[500];
	typeAxle axlesData[MAX_AXLES_TRAIN];
}typeTrainData;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	bool axlesDebugging();
	int getTotalAxles();
	void createAndSaveTrain(Fifo *aarTags);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


/***************************************************** Class speedProc **********************************************
*/
class axleProc {
public:
	typeAxle lastAxle;
	axleProc();
	void add(info * command);
	void initLastAxle();
	void updateLastAxle(typeAxle * newAxle);
};

#endif /*AXLEPROCESS_H_*/
