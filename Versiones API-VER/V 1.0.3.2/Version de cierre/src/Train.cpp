/*
 * Train.cpp
 *
 *  Created on: 20 may 2021
 *      Author: mgz
 */

#include <basictypes.h>
#include <string.h>
#include <stdlib.h>
#include "libs/Train.h"
#include "libs/TagUtilities.h"
#include "libs/Configurations.h"
#include "libs/Rtc_Controller.h"
#include "libs/Fifo/Fifo.h"
#include "libs/AxleProcess.h"

void fillAemTrain(typeT94Aem * segAem);

typeTrain * myTrain = NULL;

int trainAxlesQty = 0;

//typeTrainAdmin trainAdmin;

typeT94Aem segAemEmpty = {{'A','E','M'},{'1','0','5','5','5'},{' ',' ',' ',' ',' ',' ',' '},{0,0,0,0,0,0},{0,0,0,0},{0,0,0,0},{'0','5','0'},{'N'},{' ',' ',' '},{0,0,0,0},
						{' '},{' '},{' '},{' '},{'M'},{0,0,0},{0,0,0},{0,0,0},{0},{' '},{'F'},{' '},{' ',' ',' ',' ',' '},{' '},{0,0},{0,0},{0,0,0},{0,0,0},{0,0,0,0},
						{0},{0,0},{0,0},0,0,0,0,0,0,0,0,0,0};
typeT94Eoc segEocEmpty = {{'E','O','C'},{0,0,0,0,0,0,0,0,0,0}};
typeT94Eot segEotEmpty = {{'\0','\0','\0'}, {0,0,0}, {0}, {0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {0,0}, {0,0}, {0}, {0}, {0,0}, {0,0}, 0, 0};

uint32_t CalcTrainSize(typeTrain * train){
	uint32_t trainSize = 0;
	trainSize = sizeof( typeTrain )  - ((sizeof( typeT94Rre ) * MAX_RRE_TRAIN)) + (sizeof( typeT94Rre ) * train->qtyRre);
	return trainSize;
}


/***********Number of axles by train************/
int getTrainRreAxlesQty(){
	return trainAxlesQty;
}

/***********Number of cars by train************/
int getTrainRreQty(){
	return myTrain->qtyRre;
}

/***********Return last train created************/
typeTrain * getTrain(){
	return myTrain;
}

/***********Start the train buffer where a new train will be stored************/
void initTrain(){
	if(!myTrain){
		myTrain = (typeTrain *)malloc(sizeof (typeTrain));
	}
	typeTrain * train = myTrain;
	memcpy(& train->segAem, & segAemEmpty, sizeof(typeT94Aem));
	memcpy(& train->segEoc, & segEocEmpty, sizeof(typeT94Eoc));
	memcpy(& train->segEot, & segEotEmpty, sizeof(typeT94Eot));
	train->hasSegEot = FALSE;
	train->idxRre = 0;
	train->qtyRre = 0;
	trainAxlesQty = 0;
	train->stateOfReport = stNotReady;
	train->qtyBytes = sizeof(typeT94AemPost);
}

/***********Free the train buffer where a train was stored************
void freeTrain(typeTrain * train){
	free( train );
}*/

void DeleteLastRREFromTrain(bool subsCounters){
	typeTrain * train = myTrain;
	if(train->qtyRre > 0){

		train->qtyRre--;
		if(subsCounters){
			if(train->segRre[train->qtyRre].groupCode[0] == 'D'){
				if(train->segRre[train->qtyRre].tagStatus[0] != 'N'){
					train->segAem.iLocomotivesTagged--;
				}
				train->segAem.iLocomotiveCount--;
			}else{
				if(train->segRre[train->qtyRre].tagStatus[0] != 'N'){
					train->segAem.iRailcarsTagged--;
				}
				train->segAem.iRailcarCount--;
			}

			char axlesCount[4];
			memcpy(axlesCount, train->segRre[train->qtyRre].axlesCount, 3);
			axlesCount[3] = '\0';
			trainAxlesQty -= atoi(axlesCount);
		}
		train->qtyBytes -= sizeof(typeT94RrePost);
	}
}

/***********Add RRE to train rre's buffer************/
void addRreToTrain(typeT94Rre * segRreToSave, bool addCounters){
	typeTrain * train = myTrain;
	if(train->qtyRre < MAX_RRE_TRAIN){
		memcpy( &train->segRre[train->qtyRre++], segRreToSave, sizeof(typeT94Rre) );

		if(addCounters){
			if(segRreToSave->groupCode[0] == 'D'){
				if(segRreToSave->tagStatus[0] != 'N'){
					train->segAem.iLocomotivesTagged++;
				}
				train->segAem.iLocomotiveCount++;
			}else{
				if(segRreToSave->tagStatus[0] != 'N'){
					train->segAem.iRailcarsTagged++;
				}
				train->segAem.iRailcarCount++;
			}

			char axlesCount[4];
			memcpy(axlesCount, segRreToSave->axlesCount, 3);
			axlesCount[3] = '\0';
			trainAxlesQty += atoi(axlesCount);
		}

		train->qtyBytes += sizeof(typeT94RrePost);
	}
}

void closeTrainToSave(typeT94Aem * segAem){
	fillAemTrain(segAem);

	//MAKING THE EOC
	typeT94Eoc * segEoc = &myTrain->segEoc;
	convIntToAscii(myTrain->qtyBytes, segEoc->totalBytesCount, 10);
	segEoc->separator[0] = '&';

	//TO ADD TO EOT
	if(myTrain->hasSegEot){
		typeT94Eot * auxEot = &myTrain->segEot;
		convIntToAscii(myTrain->qtyRre, auxEot->sequenceNumber, 3);

		//JUST TO KNOW IF EOT DATE TIME IS HIGHER THAN LAST CAR DATE TIME
		if( cmpAeiTime(&auxEot->endTime, &myTrain->segRre[myTrain->qtyRre - 1].endTime) >= 0){
			copyAeiTime(&segEoc->startTime, &auxEot->endTime);
			copyAeiTime(&segEoc->endTime, &auxEot->endTime);
		}else{
			copyAeiTime(&segEoc->startTime, &myTrain->segRre[myTrain->qtyRre - 1].endTime);
			copyAeiTime(&segEoc->endTime, &myTrain->segRre[myTrain->qtyRre - 1].endTime);
		}
	}else{
		copyAeiTime(&segEoc->startTime, &myTrain->segRre[myTrain->qtyRre - 1].endTime);
		copyAeiTime(&segEoc->endTime, &myTrain->segRre[myTrain->qtyRre - 1].endTime);
	}

	//This train is ready to be reported
	myTrain->stateOfReport = stReady;
	incrementTrainCounter();
}

void fillAemTrain(typeT94Aem * segAemToSave){
	const char * temp;
	char time[24];

	typeT94Aem * segAem = &myTrain->segAem;
	segAem->iMaxSpeed = segAemToSave->iMaxSpeed;
	segAem->iMinSpeed = segAemToSave->iMinSpeed;
	segAem->iAvSpeed = segAemToSave->iAvSpeed;
	segAem->iTotalAxleCount = segAemToSave->iTotalAxleCount;
	segAem->iOneDirection = segAemToSave->iOneDirection;
	copyAeiTime( &segAem->iStartTime, &segAemToSave->iStartTime );
	copyAeiTime( &segAem->iStopTime, &segAemToSave->iStopTime );

	//segAem->iRailcarCount = segAem->iRailcarsTagged;
	//segAem->iLocomotiveCount = segAem->iLocomotivesTagged;

	temp = getAeiID();
	for(int i = 0; i < 7; i++)
		segAem->siteId[i] = temp[i];

	convertTimeToASCII( &segAem->iStartTime, time );
	segAem->startDate[0] = time[2];
	segAem->startDate[1] = time[3];
	segAem->startDate[2] = time[5];
	segAem->startDate[3] = time[6];
	segAem->startDate[4] = time[8];
	segAem->startDate[5] = time[9];

	segAem->startTime[0] = time[11];
	segAem->startTime[1] = time[12];
	segAem->startTime[2] = time[14];
	segAem->startTime[3] = time[15];

	convertTimeToASCII( &segAem->iStopTime, time);
	segAem->stopTime[0] = time[11];
	segAem->stopTime[1] = time[12];
	segAem->stopTime[2] = time[14];
	segAem->stopTime[3] = time[15];

	convIntToAscii(getTrainCounter(), segAem->trainSeqNumber, 4);

	if(segAem->iOneDirection){
		//Train in one direction
		if(segAem->iMaxSpeed > 8)
			segAem->movementStatus[0] = 'A';
		else
			segAem->movementStatus[0] = 'B';
	}
	else
		segAem->movementStatus[0] = 'C';

	segAem->directionOfTravel[0] = segAemToSave->directionOfTravel[0];

	convIntToAscii(segAem->iMaxSpeed, segAem->maxSpeed, 3);
	convIntToAscii(segAem->iMinSpeed, segAem->minSpeed, 3);
	convIntToAscii(segAem->iAvSpeed, segAem->avSpeed, 3);

	convIntToAscii(segAem->iLocomotiveCount, segAem->locomotiveCount, 2);
	convIntToAscii(segAem->iLocomotivesTagged, segAem->locomotivesTagged, 2);

	convIntToAscii(segAem->iRailcarCount, segAem->railcarCount, 3);
	convIntToAscii(segAem->iRailcarsTagged, segAem->railcarsTagged, 3);

	convIntToAscii(segAem->iTotalAxleCount, segAem->totalAxleCount, 4);
}

void fillEot(typeAarDec * tag, int * seqNumber){
	int antCount0 = 0, antCount1 = 0;
	typeT94Eot * auxEot = &myTrain->segEot;

	if(tag->antennaNumber == 0){
		antCount0 = tag->timesRead;
	}else if(tag->antennaNumber == 1){
		antCount1 = tag->timesRead;
	}

	auxEot->segmentID[0] = 'E';
	auxEot->segmentID[1] = 'O';
	auxEot->segmentID[2] = 'T';

	convIntToAscii(*seqNumber, auxEot->sequenceNumber, 3);
	auxEot->groupCode[0] = 'E';
	for(int i = 0; i < 4; i++)
		auxEot->ownerCode[i] = tag->equipmentInitial[i];
	convIntToAscii(tag->carNumber, auxEot->ownerEquipmentNumber, 10);

	if(antCount1 > 99){ antCount1 = 99; }
	if(antCount0 > 99){ antCount0 = 99; }
	convIntToAscii(antCount0, auxEot->antena0Count, 2);
	convIntToAscii(antCount1, auxEot->antena1Count, 2);
	auxEot->ant0Count = antCount0;
	auxEot->ant1Count = antCount1;

	copyAeiTime( &auxEot->startTime, &tag->timeFirstRead );
	copyAeiTime( &auxEot->endTime, &tag->timeLastRead );

	auxEot->tagDetailStatus[0] = 'K';

	auxEot->separator[0] = '&';
	if(* seqNumber > 1){
		(* seqNumber)++;
	}
	myTrain->hasSegEot = true;
}

void initTrainAdmin(){
	if(!myTrain){
		myTrain = (typeTrain *)malloc(sizeof (typeTrain));
	}
}
