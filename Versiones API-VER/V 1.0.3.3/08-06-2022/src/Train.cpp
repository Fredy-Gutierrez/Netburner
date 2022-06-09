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


typeTrain * myTrain = NULL;

//int trainAxlesQty = 0;

typeT94Aem segAemEmpty = {{'A','E','M'},{'1','0','5','5','5'},{' ',' ',' ',' ',' ',' ',' '},{0,0,0,0,0,0},{0,0,0,0},{0,0,0,0},{'0','5','0'},{'N'},{' ',' ',' '},{0,0,0,0},
						{' '},{' '},{' '},{' '},{'M'},{0,0,0},{0,0,0},{0,0,0},{0},{' '},{'F'},{' '},{' ',' ',' ',' ',' '},{' '},{0,0},{0,0},{0,0,0},{0,0,0},{0,0,0,0},
						{0},{0,0},{0,0},0,0,0,0,0,0,0,0,0,0};
typeT94Eot segEotEmpty = {{'\0','\0','\0'}, {0,0,0}, {0}, {0,0,0,0}, {0,0,0,0,0,0,0,0,0,0}, {0,0}, {0,0}, {0}, {0}, {0,0}, {0,0}, 0, 0};
typeT94Eoc segEocEmpty = {{'E','O','C'},{0,0,0,0,0,0,0,0,0,0}};
//							SEGMENTID	 #SECUENCE	E.G.CODE	OWNERCODE			EQUIPMENT NUMBER					    ORI	  RESV    AXC 	TAGS   TAGD		ANT0		ANT1		SPEED			#AXLES		#PLATFORM    #SEP1	#DIR      #LENGTH          #BC	 #TAGPCODE  #SEP2
typeT94Rre segRreEmpty = { {'R','R','E'}, {'0','0','0'}, {'?'}, {' ',' ',' ',' '}, {'0','0','0','0','0','0','0','0','0','0'}, {'U'}, {' '}, {'G'}, {'N'}, {'?'}, {'0','0'}, {'0','0'}, {'0','0','0'}, {'0','0','0'}, {'0','0','0'}, {'&'}, {'0'}, {'0','0','0','0'}, {'0'}, {'0','0'}, {'&'}};


typeT94Eot segEotAux;//Auxiliar EOT
bool hasEOTSeg = FALSE;
typeT94Rre AllRRE[400];//HAS ALL THE RRES INCLUDING THE CHANGE DIRECTIONS
int allRREQty = 0;
int auxRREIt = 0;//is moving every direction get
char firstDir = '0';
bool oneDirection = TRUE;


uint32_t CalcTrainSize(typeTrain * train){
	uint32_t trainSize = 0;
	trainSize = sizeof( typeTrain )  - ((sizeof( typeT94Rre ) * MAX_RRE_TRAIN)) + (sizeof( typeT94Rre ) * train->qtyRre);
	return trainSize;
}

/***********Number of cars by train************/
int getTrainRreQty(){
	return allRREQty;
}



void cleanTrain(typeTrain * train){
	memcpy(& train->segAem, & segAemEmpty, sizeof(typeT94Aem));
	memcpy(& train->segEoc, & segEocEmpty, sizeof(typeT94Eoc));
	memcpy(& train->segEot, & segEotEmpty, sizeof(typeT94Eot));
	train->hasSegEot = FALSE;
	train->idxRre = 0;
	train->qtyRre = 0;
	train->stateOfReport = stNotReady;
	train->qtyBytes = sizeof(typeT94AemPost);
	train->reportedFlag = false;
}

/***********Free the train buffer where a train was stored*************/
void freeTrain(typeTrain * train){
	free( train );
}

void DeleteLastRREFromTrain(bool subsCounters){
	if(allRREQty > 0){
		allRREQty--;
	}
}

/***********Add RRE to train rre's buffer************/
void addRreToTrain(typeT94Rre * segRreToSave, bool addCounters){
	if(allRREQty < MAX_RRE_TRAIN){
		if(allRREQty == 0){
			firstDir = segRreToSave->direction[0];
		}else{
			if(firstDir != segRreToSave->direction[0]){
				oneDirection = FALSE;
			}
		}
		memcpy( &AllRRE[allRREQty++], segRreToSave, sizeof(typeT94Rre) );
	}
}

int getAxlesQtyFrom(typeTrain * train){
	int axlesQty = 0;
	int rreIt = 0;
	while(rreIt < train->qtyRre){
		char axlesCar[4];
		memcpy(axlesCar, train->segRre[rreIt++].axlesCount, 3);
		axlesCar[3] = '\0';

		axlesQty += atoi(axlesCar);
	}
	return axlesQty;
}

void addCarWithSpareAxles(typeTrain * train, int qtyAxlesTotal){
	int rreqtyAxles = getAxlesQtyFrom(train);
	if(qtyAxlesTotal > rreqtyAxles){
		typeT94Rre emptyRre;
		//MAKE A RRE WITH NO TAG
		memcpy(&emptyRre, &segRreEmpty, sizeof(typeT94Rre));

		if(hasEOTSeg){
			int result = cmpAeiTime(&segEotAux.endTime, &train->segRre[train->qtyRre - 1].endTime);
			if(result >= 0){
				copyAeiTime(&emptyRre.startTime, &segEotAux.endTime);
				copyAeiTime(&emptyRre.endTime, &segEotAux.endTime);
			}else{
				copyAeiTime(&emptyRre.startTime, &train->segRre[train->qtyRre - 1].endTime);
				copyAeiTime(&emptyRre.endTime, &train->segRre[train->qtyRre - 1].endTime);
			}
		}else{
			copyAeiTime(&emptyRre.startTime, &train->segRre[train->qtyRre - 1].endTime);
			copyAeiTime(&emptyRre.endTime, &train->segRre[train->qtyRre - 1].endTime);
		}
		addAeiTime(&emptyRre.startTime, 100);
		addAeiTime(&emptyRre.endTime, 100);

		//Fill with last RRE information

		emptyRre.direction[0] = train->segRre[train->qtyRre - 1].direction[0];
		memcpy(emptyRre.speedVehicle, &train->segRre[train->qtyRre - 1].speedVehicle, 3);
		emptyRre.platformCount[2] = '1';

		convIntToAscii(1, emptyRre.ownerEquipmentNumber, 10);
		convIntToAscii(train->qtyRre + 1, emptyRre.sequenceNumber, 3);
		emptyRre.groupCode[0] = 'R';

		convIntToAscii(qtyAxlesTotal - rreqtyAxles, emptyRre.axlesCount, 3);

		memcpy( &train->segRre[ train->qtyRre++ ], &emptyRre, sizeof(typeT94Rre) );
	}
}

void fillEot(typeAarDec * tag, int * seqNumber){
	int antCount0 = 0, antCount1 = 0;

	typeT94Eot * auxEot = &segEotAux;

	if(tag->antennaNumber == 0){
		antCount0 = tag->timesRead;
	}else if(tag->antennaNumber == 1){
		antCount1 = tag->timesRead;
	}

	if(!hasEOTSeg){
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
		hasEOTSeg = TRUE;
	}else{
		char ant[3];
		memcpy(ant,auxEot->antena0Count, 2);
		ant[2]= 0;
		int a0Count = atoi(ant);

		antCount0 += a0Count;

		memcpy(ant,auxEot->antena1Count, 2);
		ant[2]= 0;
		int a1Count = atoi(ant);

		antCount1 += a1Count;

		if(antCount1 > 99){ antCount1 = 99; }
		if(antCount0 > 99){ antCount0 = 99; }

		convIntToAscii(antCount0, auxEot->antena0Count, 2);
		convIntToAscii(antCount1, auxEot->antena1Count, 2);

		copyAeiTime( &auxEot->endTime, &tag->timeLastRead );
	}

	if(* seqNumber > 1){
		(* seqNumber)++;
	}
}

void fillAem(typeTrain * train, int qtyAxles){
	int maxSpeed = 0, minSpeed = 1000, avSpeed = 0;
	int trainDir = 0;

	typeT94Aem * segAem = &train->segAem;

	for(int i = 0; i < train->qtyRre; i++){
		typeT94Rre * rre = &train->segRre[i];

		char speed[4];  memcpy(speed, rre->speedVehicle,3); speed[3] = '\0';
		int vehicleSpeed = atoi(speed);

		if(i == 0){
			if(rre->direction[0] == '1'){
				trainDir = 1;
			}
		}

		//AEM SPEED
		if(vehicleSpeed > maxSpeed)
			maxSpeed = vehicleSpeed;
		if(vehicleSpeed < minSpeed)
			minSpeed = vehicleSpeed;
		avSpeed += vehicleSpeed;

		//GET THE TAGGED AND NO TAGGED RRES
		if(rre->groupCode[0] == 'D'){
			segAem->iLocomotiveCount++;
			if(rre->tagStatus[0] != 'N'){
				segAem->iLocomotivesTagged++;
			}
		}else{
			segAem->iRailcarCount++;
			if(rre->tagStatus[0] != 'N'){
				segAem->iRailcarsTagged++;
			}
		}
	}
	segAem->iMaxSpeed = maxSpeed;
	segAem->iMinSpeed = minSpeed;
	segAem->iAvSpeed = (avSpeed / train->qtyRre);
	segAem->iTotalAxleCount = qtyAxles;

	copyAeiTime(&segAem->iStartTime, &train->segRre[0].startTime);
	copyAeiTime(&segAem->iStopTime, &train->segRre[train->qtyRre - 1].endTime);

	//MAKING AEM AS STRING
	if(trainDir == 1) segAem->directionOfTravel[0] = 'S'; else segAem->directionOfTravel[0] = 'N';

	const char * aeiID = getAeiID();
	for(int i = 0; i < 7; i++){
		if(aeiID[i] == 0){
			segAem->siteId[i] = ' ';
		}else{
			segAem->siteId[i] = aeiID[i];
		}
	}

	char time[24];
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

	if(oneDirection){
		if(segAem->iMaxSpeed >= 5){
			segAem->movementStatus[0] = 'A';
		}else{
			segAem->movementStatus[0] = 'B';
		}
	}else{
		segAem->movementStatus[0] = 'D';
	}

	convIntToAscii(segAem->iMaxSpeed, segAem->maxSpeed, 3);
	convIntToAscii(segAem->iMinSpeed, segAem->minSpeed, 3);
	convIntToAscii(segAem->iAvSpeed, segAem->avSpeed, 3);

	convIntToAscii(segAem->iLocomotiveCount, segAem->locomotiveCount, 2);
	convIntToAscii(segAem->iLocomotivesTagged, segAem->locomotivesTagged, 2);

	convIntToAscii(segAem->iRailcarCount, segAem->railcarCount, 3);
	convIntToAscii(segAem->iRailcarsTagged, segAem->railcarsTagged, 3);

	convIntToAscii(segAem->iTotalAxleCount, segAem->totalAxleCount, 4);
}

void fixedRRETrain(typeTrain * train){
	int seqNum = 1;
	typeAeiTime * lastRRETime;
	for (int it = 0; it < train->qtyRre; it++) {
		typeT94Rre * rre = &train->segRre[it];
		if(it > 0){
			//THE MULTITAGGED CAR HAS THE SAME TIMES SO THE SECUENCE MUST BE THE SAME
			int cmpResult = cmpAeiTime(lastRRETime, &rre->startTime);
			if(cmpResult == 0){
				seqNum--;
				convIntToAscii(seqNum ,rre->sequenceNumber, 3);
				seqNum++;
			}else{
				convIntToAscii(seqNum++ ,rre->sequenceNumber, 3);
			}
		}else{
			convIntToAscii(seqNum++ ,rre->sequenceNumber, 3);
		}

		lastRRETime = &rre->startTime;

		//PUT THE EOT SEGMENT IN CORRECT POSITION
		if(hasEOTSeg){
			if(it + 1 < train->qtyRre){
				int actual = cmpAeiTime( &segEotAux.startTime, &rre->startTime);
				int next = cmpAeiTime( &segEotAux.startTime, &train->segRre[it + 1].startTime);
				if(actual >= 0 && next <= 0){
					train->hasSegEot = true;
					memcpy(segEotAux.sequenceNumber, rre->sequenceNumber, 3 );
					memcpy(&train->segEot, &segEotAux, sizeof(typeT94Eot));
					hasEOTSeg = false;
				}else if(actual < 0){//if EOT is the first lecture before the first axle car
					train->hasSegEot = true;
					memcpy(segEotAux.sequenceNumber, rre->sequenceNumber, 3 );
					memcpy(&train->segEot, &segEotAux, sizeof(typeT94Eot));
					hasEOTSeg = false;
				}
			}else{
				int actual = cmpAeiTime( &segEotAux.startTime, &rre->startTime);
				if(actual >= 0){
					if( auxRREIt < allRREQty  ){
						int next = cmpAeiTime( &segEotAux.startTime, &AllRRE[auxRREIt].startTime);
						if(next <= 0){
							train->hasSegEot = true;
							memcpy(segEotAux.sequenceNumber, rre->sequenceNumber, 3 );
							memcpy(&train->segEot, &segEotAux, sizeof(typeT94Eot));
							hasEOTSeg = false;
						}
					}else{
						train->hasSegEot = true;
						memcpy(segEotAux.sequenceNumber, rre->sequenceNumber, 3 );
						memcpy(&train->segEot, &segEotAux, sizeof(typeT94Eot));
						hasEOTSeg = false;
					}
				}
			}
		}
	}
}

void fillEoc(typeTrain * train){
	typeT94Eoc * segEoc = &train->segEoc;
	train->qtyBytes += train->qtyRre * sizeof(typeT94Rre);

	//TO ADD TO EOT
	if(train->hasSegEot){
		typeT94Eot * auxEot = &train->segEot;
		train->qtyBytes += sizeof(typeT94Eot);
		//JUST TO KNOW IF EOT DATE TIME IS HIGHER THAN LAST CAR DATE TIME
		if( cmpAeiTime(&auxEot->endTime, &train->segRre[train->qtyRre - 1].endTime) >= 0){
			copyAeiTime(&segEoc->startTime, &auxEot->endTime);
			copyAeiTime(&segEoc->endTime, &auxEot->endTime);
		}else{
			copyAeiTime(&segEoc->startTime, &train->segRre[train->qtyRre - 1].endTime);
			copyAeiTime(&segEoc->endTime, &train->segRre[train->qtyRre - 1].endTime);
		}
	}else{
		copyAeiTime(&segEoc->startTime, &train->segRre[train->qtyRre - 1].endTime);
		copyAeiTime(&segEoc->endTime, &train->segRre[train->qtyRre - 1].endTime);
	}

	addAeiTime(&segEoc->startTime, 100);
	addAeiTime(&segEoc->endTime, 100);

	convIntToAscii(train->qtyBytes, segEoc->totalBytesCount, 10);
	segEoc->separator[0] = '&';
}
void fillTrain(typeTrain * newTrain, bool useRREAx, int qtyAxles){
	if(useRREAx){
		qtyAxles = getAxlesQtyFrom(newTrain);
	}else{//profiler not work
		addCarWithSpareAxles(newTrain, qtyAxles);
	}

	fillAem(newTrain, qtyAxles);

	fixedRRETrain(newTrain);

	fillEoc(newTrain);

	newTrain->stateOfReport = stReady;

	incrementTrainCounter();
}

/*********************DIVIDE THE RRES TRAIN IN DIRECTIONS BLOCKS WHEN IT HAS MORE THAN ONE DIRECTION******************/
typeTrain * getNextTrainFromRREs(bool useRREAx = true, int qtyAxles = 0){
	if(auxRREIt < allRREQty){
		typeTrain * newTrain = (typeTrain *)malloc(sizeof (typeTrain));
		cleanTrain(newTrain);
		if(oneDirection){
			while(auxRREIt < allRREQty){
				memcpy( &newTrain->segRre[newTrain->qtyRre++], &AllRRE[auxRREIt++], sizeof(typeT94Rre) );
			}
		}else{
			bool sameDir = TRUE;
			char firstDir = '0';
			while(auxRREIt < allRREQty && sameDir){
				if(newTrain->qtyRre == 0){
					firstDir = AllRRE[auxRREIt].direction[0];
					memcpy( &newTrain->segRre[newTrain->qtyRre++], &AllRRE[auxRREIt++], sizeof(typeT94Rre) );
				}else{
					if(firstDir == AllRRE[auxRREIt].direction[0]){
						memcpy( &newTrain->segRre[newTrain->qtyRre++], &AllRRE[auxRREIt++], sizeof(typeT94Rre) );
					}else{
						sameDir = FALSE;
					}
				}
			}
		}

		fillTrain(newTrain, useRREAx, qtyAxles);

		return newTrain;
	}else{
		return NULL;
	}
}

/***********Return last train created************/
typeTrain * getTrain(){
	if(oneDirection){
		int totalAxles = getTotalAxles();
		if( totalAxles > 0 ){//profiler not work
			return getNextTrainFromRREs(false, totalAxles);
		}else{
			return getNextTrainFromRREs();
		}
	}else{
		return getNextTrainFromRREs();
	}
}

/***********Start the train buffer where a new train will be stored************/
void newTrainProcess(){
	allRREQty = 0;
	oneDirection = TRUE;
	hasEOTSeg = FALSE;
	auxRREIt = 0;
}

void initTrainAdmin(){
	allRREQty = 0;
	oneDirection = TRUE;
	hasEOTSeg = FALSE;
	auxRREIt = 0;
}
