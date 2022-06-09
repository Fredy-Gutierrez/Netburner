/*
 * AxleProcess.cpp
 *
 *  Created on: 25 april 2021
 *      Author: Integra MGZ
 */


#include <basictypes.h>
#include <libs/TagUtilities.h>
#include <stdlib.h>
#include <string.h>
#include "libs/log.h"
#include "libs/Configurations.h"
#include "libs/Rtc_Controller.h"
#include "libs/IO_Interface.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"
#include "libs/Fifo/Fifo.h"
#include "libs/AxleProcess.h"
#include "libs/TagProcessor.h"
#include "libs/Report2Server.h"
#include "libs/Train.h"
#include "libs/DBTrainDriver.h"

Fifo axles;
Fifo aarTags;

typeTrainData * trainData;

//typeAxle axlesTrain[MAX_AXLES_TRAIN];
//static int qtyAxles;

static int qtyAxlesTag;
static int qtyCars;
//static int qtySegments;

typeT94Rre profileRRE[MAX_RRE_TRAIN];
int profileRREQty = 0;
typeT94Rre tagsRRE[MAX_RRE_TRAIN];
int tagsRREQty = 0;


//							SEGMENTID	 #SECUENCE	E.G.CODE	OWNERCODE			EQUIPMENT NUMBER					    ORI	  RESV    AXC 	TAGS   TAGD		ANT0		ANT1		SPEED			#AXLES		#PLATFORM    #SEP1	#DIR      #LENGTH          #BC	 #TAGPCODE  #SEP2
typeT94Rre emptyRRE = { {'R','R','E'}, {'0','0','0'}, {'?'}, {' ',' ',' ',' '}, {'0','0','0','0','0','0','0','0','0','0'}, {'U'}, {' '}, {'G'}, {'N'}, {'?'}, {'0','0'}, {'0','0'}, {'0','0','0'}, {'0','0','0'}, {'0','0','0'}, {'&'}, {'0'}, {'0','0','0','0'}, {'0'}, {'0','0'}, {'&'}};

axleProc::axleProc(){
	initLastAxle();
}

void axleProc::add(info * command){
	typeAxle * newAxle = (typeAxle *)malloc(sizeof (typeAxle));
	int difTime;
	
	copyAeiTime(&newAxle->time, &command->chgTime);
	newAxle->speed = (int)command->par1;
	newAxle->dir = (int)command->par2;
	newAxle->carNumber = -1;
	
	//Calculates difference with previous axle
	if(lastAxle.dir != newAxle->dir || axles.FifoSize() <= 0){
		newAxle->distance = -1;	//indicates there was no previous axle
	}else{
		difTime = difAeiTime(&lastAxle.time, &newAxle->time);
		newAxle->distance = ( (newAxle->speed + lastAxle.speed) * difTime ) / 720;
	}
	updateLastAxle(newAxle);
	axles.Add((void *) newAxle);
}

void axleProc::initLastAxle(){
	lastAxle.speed = -1;
	lastAxle.dir = 0;
}

void axleProc::updateLastAxle(typeAxle * newAxle){
	copyAeiTime(& lastAxle.time, & newAxle->time);
	lastAxle.speed = newAxle->speed;
	lastAxle.dir = newAxle->dir;
	lastAxle.distance = newAxle->distance;
}

uint32_t CalcTrainDataSize(typeTrainData * trainData){
	uint32_t trainSize = 0;
	trainSize = sizeof( typeTrainData )  - ((sizeof( typeAxle ) * MAX_AXLES_TRAIN)) + (sizeof( typeAxle ) * trainData->qtyAxles);
	return trainSize;
}

void printT94RreCar(typeT94Rre *carPrn, bool start, int index){
	char asciiFormat[30];
	char asciiFormatFin[30];
	char segment[50];
	memcpy(segment, carPrn, 49);
	segment[49] = 0;

	if(start){
		convertTimeToASCII(& carPrn->startTime, asciiFormat);
	}else{
		convertTimeToASCII(& carPrn->endTime, asciiFormat);
	}
	convertTimeToASCII(& carPrn->endTime, asciiFormatFin);
	//sprintf(strPrnn, "\n%02d) RRE car decoded: %s  %s", index, carSegment->segmentRre, asciiFormat);logg.prn(strPrnn);
	logg.newPrint("\n%02d) RRE car decoded: %s  %s  %s", index, segment, asciiFormat, asciiFormatFin);
}

void printT94RreMatchError(typeT94Rre *carPrn, int index){
	char asciiFormat[30];
	char asciiFormatFin[30];

	char segment[50];
	memcpy(segment, carPrn, 49);
	segment[49] = 0;

	convertTimeToASCII(& carPrn->startTime, asciiFormat);
	convertTimeToASCII(& carPrn->endTime, asciiFormatFin);
	logg.newPrint("\n%02d) RRE NOT MATCHED: %s  %s  %s", index, segment, asciiFormat, asciiFormatFin);
}

void printAxle(typeAxle *axle, int index){
	char asciiFormat[30];
	convertTimeToASCII(& axle->time, asciiFormat);
	if(axle->dir==1)
		logg.newPrint("\n%02d) Axle: speed %d Dm/h, direction FORWARD . Distance to previous %d cm.  %s", index, axle->speed, axle->distance, asciiFormat);
	else
		logg.newPrint("\n%02d) Axle: speed %d Dm/h, direction BACKWARD . Distance to previous %d cm.  %s", index, axle->speed, axle->distance, asciiFormat);
}

void setToTrainRRETags(){
	logg.newPrint("\nUSING RRES' TAGS TO MAKE TRAIN");
	for(int i = 0; i < tagsRREQty; i++){
		addRreToTrain( &tagsRRE[i] );
	}
}

void copyRRETagTo(typeT94Rre * axleRre, typeT94Rre * tagRre, int * rreSeq, int *eqNumber = NULL, bool useTagEqNum = true){
	//RRE NUMBER
	convIntToAscii(*rreSeq, axleRre->sequenceNumber, 3);
	//Group Code
	if(tagRre->groupCode[0] != '?' && tagRre->groupCode[0] != axleRre->groupCode[0]){
		char axlesCount[4];
		memcpy(axlesCount, axleRre->axlesCount, 3);
		axlesCount[3] = '\0';
		int axlesQty = atoi( axlesCount );
		if(axlesQty == 4){
			if(axleRre->groupCode[0] == 'R'){
				axleRre->groupCode[0] = tagRre->groupCode[0];
			}
		}else if(axlesQty % 2 != 0){
			axleRre->groupCode[0] = tagRre->groupCode[0];
		}
	}
	//OwnerCode
	memcpy(axleRre->ownerCode, tagRre->ownerCode, 4);
	//OwnerEquipmentNumber
	if(useTagEqNum){
		memcpy(axleRre->ownerEquipmentNumber, tagRre->ownerEquipmentNumber, 10);
	}else{
		convIntToAscii(*eqNumber, axleRre->ownerEquipmentNumber, 10);
		(*eqNumber)++;
	}
	//Orientation
	memcpy(axleRre->orientation, tagRre->orientation, 1);
	//TagStatus
	memcpy(axleRre->tagStatus, tagRre->tagStatus, 1);
	//TagDetailStatus
	memcpy(axleRre->tagDetailStatus, tagRre->tagDetailStatus, 1);
	//Antena0Count
	memcpy(axleRre->antena0Count, tagRre->antena0Count, 2);
	//Antena1Count
	memcpy(axleRre->antena1Count, tagRre->antena1Count, 2);
	//PlatformCount
	//memcpy(axleRre->platformCount, tagRre->platformCount, 3);//PLATFORM CODE IS ACORDDING TO AXLES
	//Separator1
	memcpy(axleRre->separator1, tagRre->separator1, 1);
	//Direction
	//memcpy(axleRre->direction, tagRre->direction, 1);//DIRECTION IS ACORDDING TO AXLES
	//Length
	memcpy(axleRre->length, tagRre->length, 4);
	//TagBearingCode
	memcpy(axleRre->tagBearingCode, tagRre->tagBearingCode, 1);
	//TagPlatformCode
	memcpy(axleRre->tagPlatformCode, tagRre->tagPlatformCode, 2);
	//Separator2
	memcpy(axleRre->separator2, tagRre->separator2, 1);

	axleRre->axlesCountTag = tagRre->axlesCountTag;
	axleRre->carNumberTag = tagRre->carNumberTag;
	axleRre->speedTag = tagRre->speedTag;
	axleRre->ant0Count = tagRre->ant0Count;
	axleRre->ant1Count = tagRre->ant1Count;

	(*rreSeq)++;
}

/**********************************MATCH THE RRES FROM TAGS RRES AND AXLES RRES******************************/
void matchRREs(){
	int noTagRRE = 1;
	int RRECount = 1;
	int itRRETag = 0, itRREAxle = 0;//Iterators for RRETags and RREAxles
	bool matching = true;

	if((profileRREQty == 0) || (tagsRREQty == 0)){//close the while but is necesary to add 1to iterators?
		matching = false;
	}

	while(matching){
		typeT94Rre * RRETag = &tagsRRE[itRRETag];
		typeT94Rre * RREAxle = &profileRRE[itRREAxle];

		int TITvsTIA = cmpAeiTime(&RRETag->startTime, &RREAxle->startTime);
		if(TITvsTIA < 0){//CASE 1
			if(/*itRRETag == 0 && */itRREAxle == 0){//if is the first matching CASE 1.i
				int TFTvsTIA = cmpAeiTime(&RRETag->endTime, &RREAxle->startTime);
				if(TFTvsTIA < 0){//CASE 1.i.i
					itRRETag++;
					printT94RreMatchError(RRETag, itRRETag);
				}else if(TFTvsTIA >= 0){//CASE 1.i.ii y CASE 1.i.iii
					int TFTvsTFA = cmpAeiTime(&RRETag->endTime, &RREAxle->endTime);
					if(TFTvsTFA <= 0){/***************CASE 1.i.ii**************/
						//match code
						copyRRETagTo(RREAxle, RRETag, &RRECount);
						itRRETag++;
						itRREAxle++;
						/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
						addRreToTrain(RREAxle);
					}else if(TFTvsTFA > 0){/***************CASE 1.i.iii**************/
						if((itRREAxle + 1) < profileRREQty){
							typeT94Rre * NextRREAxle = &profileRRE[itRREAxle+1];
							if(RREAxle->direction[0] != NextRREAxle->direction[0]){//there is a change direction CASE 1.ii.2.a
								copyRRETagTo(RREAxle, RRETag, &RRECount);
								//itRRETag++;
								itRREAxle++;
								/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
								addRreToTrain(RREAxle);
							}else{//CASE 1.ii.2.b
								//CASE 1.ii.2.b.i
								int difActualTFTvsTFA = difAeiTime(&RREAxle->endTime, &RRETag->endTime);
								int difNextTFTvsTIA = difAeiTime( &RRETag->endTime, &NextRREAxle->startTime);
								if(difActualTFTvsTFA < difNextTFTvsTIA){//if actual ACTUALRREAXLES differences is lower than NEXTRREAXLES CASE 1.ii.2.b.ii
									//match code
									copyRRETagTo(RREAxle, RRETag, &RRECount);
									itRRETag++;
									itRREAxle++;
									/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
									addRreToTrain(RREAxle);
								}else{//CASE 1.ii.2.b.iii
									itRREAxle++;
									/*************************************CASE FOR NO TAGGED**************************/
									copyRRETagTo(RREAxle, &emptyRRE, &RRECount,&noTagRRE,false);

									/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
									addRreToTrain(RREAxle);
								}
							}
						}else{
							copyRRETagTo(RREAxle, RRETag, &RRECount);
							itRRETag++;
							itRREAxle++;//sure?
							matching = false;
							/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
							addRreToTrain(RREAxle);
						}
					}
				}
			}else{//CASE NOT THE FIRST RRE큦
				typeT94Rre * PrevRREAxle = &profileRRE[itRREAxle - 1];
				if(RREAxle->direction[0] != PrevRREAxle->direction[0]){//there is a change direction CASE 1.ii.i
					int TFTvsTIA = cmpAeiTime(&RRETag->endTime, &RREAxle->startTime);
					if(TFTvsTIA < 0){//CASE 1.ii.i.1
						itRRETag++;
						printT94RreMatchError(RRETag, itRRETag);
					}else if(TFTvsTIA >= 0){//CASE 1.ii.i.2 AND CASE 1.ii.i.3
						int TFTvsTFA = cmpAeiTime(&RRETag->endTime, &RREAxle->endTime);
						if(TFTvsTFA <= 0){//1.ii.i.2
							//match code
							copyRRETagTo(RREAxle, RRETag, &RRECount);
							itRRETag++;
							itRREAxle++;
							/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
							addRreToTrain(RREAxle);
						}else if(TFTvsTFA > 0){//CASE 1.ii.i.3
							if((itRREAxle + 1) < profileRREQty){
								typeT94Rre * NextRREAxle = &profileRRE[itRREAxle+1];
								int difActualTFTvsTFA = difAeiTime(&RREAxle->endTime, &RRETag->endTime);
								int difNextTFTvsTIA = difAeiTime( &RRETag->endTime, &NextRREAxle->startTime);
								if(difActualTFTvsTFA < difNextTFTvsTIA){//if actual ACTUALRREAXLES differences is lower than NEXTRREAXLES CASE CASE 1.ii.i.3.a.i
									//match code
									copyRRETagTo(RREAxle, RRETag, &RRECount);
									itRRETag++;
									itRREAxle++;
									/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
									addRreToTrain(RREAxle);
								}else{//CASE 1.ii.i.3.a.ii
									itRREAxle++;
									/*************************************CASE FOR NO TAGGED**************************/
									copyRRETagTo(RREAxle, &emptyRRE, &RRECount,&noTagRRE,false);
									/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
									addRreToTrain(RREAxle);
								}

							}else{
								/******************************I'M NOT SURE IF THIS IS A GOOD IDEA****************************/
								copyRRETagTo(RREAxle, RRETag, &RRECount);
								itRRETag++;
								itRREAxle++;//sure?
								matching = false;
								/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
								addRreToTrain(RREAxle);
							}
						}
					}
				}else{//CASE 1.ii.ii
					int difActualTITvsTIA = difAeiTime(&RRETag->startTime, &RREAxle->startTime);
					int difPrevTITvsTFA = difAeiTime( &RRETag->startTime, &PrevRREAxle->endTime);//?
					if(difActualTITvsTIA < difPrevTITvsTFA){//if actual ACTUALRREAXLES differences is lower than NEXTRREAXLES CASE 1.ii.ii.2
						//match code
						copyRRETagTo(RREAxle, RRETag, &RRECount);
						itRRETag++;
						itRREAxle++;
						/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
						addRreToTrain(RREAxle);
					}else{//CASE 1.ii.ii.3
						/*************************************CASE FOR RRETAG MUST BE IN THE PREVIOS RRE**************************/
						if(PrevRREAxle->tagStatus[0] == 'N'){
							RRECount--;
							noTagRRE--;
							copyRRETagTo(PrevRREAxle, RRETag, &RRECount);
							/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
							DeleteLastRREFromTrain();//We must delete the last RRE form train because this has a empty RRE
							addRreToTrain(PrevRREAxle);
						}else{
							if( difPrevTITvsTFA <= 1000 || ( cmpAeiTime(&PrevRREAxle->endTime, &RRETag->startTime) >= 0)){
								char owner[5];
								char number[11];
								memcpy(owner, PrevRREAxle->ownerCode, 4);
								memcpy(number, PrevRREAxle->ownerEquipmentNumber, 10);
								owner[4] = '\0';
								number[10] = '\0';

								char ownerTag[5];
								char numberTag[11];
								memcpy(ownerTag, RRETag->ownerCode, 4);
								memcpy(numberTag, RRETag->ownerEquipmentNumber, 10);
								ownerTag[4] = '\0';
								numberTag[10] = '\0';

								if( (strcmp(owner, ownerTag) != 0) || (strcmp(number, numberTag) != 0) ){
									logg.newPrint("\n **********************CASE FOR AXLES CAR HAS TWO RRE TAG*************************");

									if(PrevRREAxle->tagStatus[0] != 'M'){
										DeleteLastRREFromTrain(FALSE);//We must delete the last RRE form train because this has a RRE

										PrevRREAxle->tagStatus[0] = 'M';
										addRreToTrain(PrevRREAxle, FALSE);
									}

									RRECount--;
									copyRRETagTo(PrevRREAxle, RRETag, &RRECount);
									PrevRREAxle->tagStatus[0] = 'M';
									addRreToTrain(PrevRREAxle, FALSE);
								}
							}
						}
						itRRETag++;
					}
				}
			}
		}else if(TITvsTIA >= 0){/*****************************************CASE 2 AND 3***********************************/
			int TITvsTFA = cmpAeiTime(&RRETag->startTime, &RREAxle->endTime);
			if(TITvsTFA <= 0){//case 2
				int TFTvsTFA = cmpAeiTime(&RRETag->endTime, &RREAxle->endTime);
				if(TFTvsTFA <= 0){//CASE 2.i
					//match code
					copyRRETagTo(RREAxle, RRETag, &RRECount);
					itRRETag++;
					itRREAxle++;
					/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
					addRreToTrain(RREAxle);
				}else{//CASE 2.ii
					if((itRREAxle + 1) < profileRREQty){
						typeT94Rre * NextRREAxle = &profileRRE[itRREAxle+1];
						if(RREAxle->direction[0] != NextRREAxle->direction[0]){//there is a change direction CASE 2.ii.i
							copyRRETagTo(RREAxle, RRETag, &RRECount);
							//itRRETag++;
							itRREAxle++;
							/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
							addRreToTrain(RREAxle);
						}else{//CASE 2.ii.ii
							int difActualTFTvsTFA = difAeiTime(&RREAxle->endTime, &RRETag->endTime);
							int difNextTFTvsTIA = difAeiTime( &RRETag->endTime, &NextRREAxle->startTime);
							if(difActualTFTvsTFA < difNextTFTvsTIA){//if actual ACTUALRREAXLES differences is lower than NEXTRREAXLES CASE CASE 2.ii.ii.2
								//match code
								copyRRETagTo(RREAxle, RRETag, &RRECount);
								itRRETag++;
								itRREAxle++;
								/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
								addRreToTrain(RREAxle);
							}else{//CASE 2.ii.ii.3
								itRREAxle++;
								/*************************************CASE FOR NO TAGGED**************************/
								copyRRETagTo(RREAxle, &emptyRRE, &RRECount,&noTagRRE,false);
								/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
								addRreToTrain(RREAxle);
							}
						}
					}else{
						copyRRETagTo(RREAxle, RRETag, &RRECount);
						itRRETag++;
						itRREAxle++;//sure?
						matching = false;
						/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
						addRreToTrain(RREAxle);
					}
				}
			}else{/*****************************************CASE 3***********************************/
				itRREAxle++;
				/*************************************CASE FOR NO TAGGED**************************/
				copyRRETagTo(RREAxle, &emptyRRE, &RRECount,&noTagRRE,false);
				/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
				addRreToTrain(RREAxle);
			}
		}

		if(!(itRREAxle < profileRREQty) || !(itRRETag < tagsRREQty)){//close the while but is necesary to add 1to iterators?
			matching = false;
		}
	}

	/****************************PRINT THE REST OF RRETAGS THAT COULD NOT BE MATCHED*******************************/
	int auxItRRETag = itRRETag;
	while(auxItRRETag < tagsRREQty){
		typeT94Rre * RRETag = &tagsRRE[auxItRRETag++];
		printT94RreMatchError(RRETag, auxItRRETag);
	}

	/****************************SET TO RREAXLES THAT COULD NOT BE MATCH A RRETAG EMPTY*******************************/
	int auxItRREAx = itRREAxle;
	while(auxItRREAx < profileRREQty){
		typeT94Rre * RREAxle = &profileRRE[auxItRREAx++];
		copyRRETagTo(RREAxle, &emptyRRE, &RRECount, &noTagRRE, false);
		/**********************************CODE TO ADD TO TRAIN THE RRE**************************************/
		addRreToTrain(RREAxle);
	}

	/****************************PRINT ALL RREAXLES*******************************/
	int itPrintAx = 0;
	while(itPrintAx < profileRREQty){
		typeT94Rre * axleRre = &profileRRE[itPrintAx++];
		printT94RreCar(axleRre, true, itPrintAx);
	}
}

/**********************************MAKE THE RRES (CARS) FROM AXLES JUST A BLOCK TRAIN DIRECTIONS******************************/
int processBlockToRRE(typeAxle *blockAxles, int qtyAxles){
	/**********************CONSTANTS TO MAKE THE PROFILER CARS********************/
	const int distanceRange = 350;//310 before
	const int fourPositionRange = 750;

	if(qtyAxles > 2){
		int axlesIt = 0;//CONTROLS THE AXLES BUFFER ITERATION
		bool DeleteRK = true;//MAKE THE WHILE REPEAT UNTIL THERE ARE NOT RAILKINGS IN AXLES BUFFER

		//DELETES THE RAILKING FROM AXLES DIRECTION BUFFER
		while(DeleteRK){
			if((axlesIt+ 2) < qtyAxles){
				//DISTANCES FOR RAILKING DISTANCE BETWEEN AXLES
				if (((blockAxles[axlesIt].distance <= 0) || (blockAxles[axlesIt].distance >= 1500)) && ((blockAxles[axlesIt+ 2].distance >= 65 && blockAxles[axlesIt+ 2].distance <= 125) || (blockAxles[axlesIt+ 2].distance >= 1500)) && ((blockAxles[axlesIt+ 1].distance >= 110 && blockAxles[axlesIt + 1].distance <= 245) || (blockAxles[axlesIt+ 1].distance >= 270 && blockAxles[axlesIt + 1].distance <= 305)))
				{
					//axlesIt =+ 2;
					/*we will delete the RAILKING axles*/
					typeAxle auxAxlesForDel[qtyAxles];
					int  newAxles = 0;
					for(int itAxles = 2; itAxles < qtyAxles; itAxles++){
						memcpy(&auxAxlesForDel[newAxles++], &blockAxles[itAxles], sizeof(typeAxle));
					}
					for(int itNewAxles = 0; itNewAxles < newAxles; itNewAxles++){
						memcpy(&blockAxles[itNewAxles], &auxAxlesForDel[itNewAxles], sizeof(typeAxle));
					}
					qtyAxles = newAxles;
				}else{
					DeleteRK = false;
				}
			}else{
				DeleteRK = false;
			}
		}
		/**************************WHAT TO DO IN IMPAR CASE***************************/
		if(qtyAxles % 2 != 0){
			typeAxle auxAxlesForDel[qtyAxles];
			int  newAxles = 0;
			for(int itAxles = 0; itAxles < qtyAxles; itAxles++){
				if(blockAxles[itAxles].distance < 10 ||blockAxles[itAxles].distance >= 100){
					memcpy(&auxAxlesForDel[newAxles++], &blockAxles[itAxles], sizeof(typeAxle));
				}
			}
			if(qtyAxles > newAxles){
				for(int itNewAxles = 0; itNewAxles < newAxles; itNewAxles++){
					memcpy(&blockAxles[itNewAxles], &auxAxlesForDel[itNewAxles], sizeof(typeAxle));
				}
			}
			qtyAxles = newAxles;
		}

		typeAxle auxAxlesData[14];//STORES JUST THE NECESSARY AXLES TO BE ANALIZED
		bool gettingCars = true;//CONTROLS THE WHILE UNTIL THE AXLES BE <= 4
		int badFormed = 0;//

		while ( gettingCars )
		{
			 int carAxles = 0;//STORES THE QUANTITY OF AXLES THAT THE PROFILER DETERMINATED
			 int checkSum = 0;//AUXILIAR VARIABLE WICH WILL HELP US TO DETERMINATE HOW MANY AXLES HAS A CAR
			 if ((axlesIt + 14) <= qtyAxles )
			 {
				 for (int it = 0; it < 14; it++)
				 {
					 memcpy(&auxAxlesData[it], &blockAxles[axlesIt + it],sizeof(typeAxle));
				 }
				 if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 8)
				 {
					 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
						 badFormed++;
					 }
					 checkSum += 1;
					 if ( (auxAxlesData[4].distance > distanceRange || auxAxlesData[4].distance == 0) && (auxAxlesData[4].distance > fourPositionRange || auxAxlesData[4].distance == 0 ) && auxAxlesData[4].speed > 8)
					 {
						 checkSum += 3;
						 if ( (auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 8)
						 {
							 checkSum += 4;
							 if ((auxAxlesData[8].distance > fourPositionRange || auxAxlesData[8].distance == 0) && auxAxlesData[8].speed > 8)
							 {
								 checkSum += 5;
								 if ((auxAxlesData[10].distance > fourPositionRange || auxAxlesData[10].distance == 0) && auxAxlesData[10].speed > 8)
								 {
									 checkSum += 8;
									 if (auxAxlesData[13].distance > distanceRange  && auxAxlesData[13].speed > 8)
									 {
										 checkSum += 11;
									 }
								 }
								 else if (auxAxlesData[11].distance > distanceRange  && auxAxlesData[11].speed > 8)
								 {
									 checkSum += 10;
								 }
							 }
							 else if (auxAxlesData[9].distance > distanceRange && auxAxlesData[9].speed > 8)
							 {
								 checkSum += 9;
							 }
						 }
						 else if (auxAxlesData[7].distance > distanceRange && auxAxlesData[7].speed > 8)
						 {
							 checkSum += 7;
						 }
					 }else if (auxAxlesData[5].distance > distanceRange && auxAxlesData[5].speed > 8)
					 {
						 checkSum += 6;
					 }
				 }
				 else if ((auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 8)
				 {
					 checkSum += 2;
					 if ( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 8)
					 {
						 checkSum += 7;
					 }
				 }
				 else
				 {
					 badFormed++;
					 checkSum += 1;
				 }
			 }else{
				 int lastAxles = qtyAxles - axlesIt;
                 for (int it = 0; it < lastAxles; it++)
                 {
                	 memcpy(&auxAxlesData[it], &blockAxles[axlesIt + it],sizeof(typeAxle));
                     //auxAxlesData[it] = blockAxles[axlesIt + it];
                 }
                 if ( lastAxles > 4 )
                 {
                     if (lastAxles > 5)
                     {
                         if (lastAxles > 6)
                         {
                             if (lastAxles > 7)
                             {
                                 if (lastAxles > 8)
                                 {
                                     if ( lastAxles > 9)
                                     {
                                         if (lastAxles > 10)
                                         {
                                             if (lastAxles > 11)
                                             {
                                                 if (lastAxles > 12)//13 axles
                                                 {
                                                     if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                                     {
                                                    	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
															 badFormed++;
														 }
                                                         checkSum += 1;
                                                         if ( (((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                                         {
                                                             checkSum += 3;
                                                             if ((auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                                             {
                                                                 checkSum += 4;
                                                                 if ( (auxAxlesData[8].distance > fourPositionRange || auxAxlesData[8].distance == 0) && auxAxlesData[8].speed > 5)
                                                                 {
                                                                     checkSum += 5;
                                                                     if ( (auxAxlesData[10].distance > fourPositionRange || auxAxlesData[10].distance == 0) && auxAxlesData[10].speed > 5)
                                                                     {
                                                                         checkSum += 8;
                                                                     }
                                                                     else if ( (auxAxlesData[11].distance > distanceRange || auxAxlesData[11].distance == 0) && auxAxlesData[11].speed > 5)
                                                                     {
                                                                         checkSum += 10;
                                                                     }
                                                                 }
                                                                 else if ( (auxAxlesData[9].distance > distanceRange || auxAxlesData[9].distance == 0) && auxAxlesData[9].speed > 5 )
                                                                 {
                                                                     checkSum += 9;
                                                                 }
                                                             }
                                                             else if ( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 5)
                                                             {
                                                                 checkSum += 7;
                                                             }
                                                         }
                                                         else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                                         {
                                                             checkSum += 6;
                                                         }
                                                     }
                                                     else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                                     {
                                                         checkSum += 2;
                                                     }
                                                     else
                                                     {
                                                    	 badFormed++;
                                                         checkSum += 1;
                                                     }
                                                 }
                                                 else//12 axles
                                                 {
                                                     if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                                     {
                                                    	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
															 badFormed++;
														 }
                                                         checkSum += 1;
                                                         if ((((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                                         {
                                                             checkSum += 3;
                                                             if ( (auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                                             {
                                                                 checkSum += 4;
                                                                 if ( (auxAxlesData[8].distance > fourPositionRange || auxAxlesData[8].distance == 0) && auxAxlesData[8].speed > 5)
                                                                 {
                                                                     checkSum += 5;
                                                                     if ( (auxAxlesData[10].distance > fourPositionRange || auxAxlesData[10].distance == 0) && auxAxlesData[10].speed > 5)
                                                                     {
                                                                         checkSum += 8;
                                                                     }
                                                                     else if( (auxAxlesData[11].distance > distanceRange || auxAxlesData[11].distance == 0) && auxAxlesData[11].speed > 5)
                                                                     {
                                                                         checkSum += 10;
                                                                     }
                                                                 }
                                                                 else if ( (auxAxlesData[9].distance > distanceRange || auxAxlesData[9].distance == 0) && auxAxlesData[9].speed > 5)
                                                                 {
                                                                     checkSum += 9;
                                                                 }
                                                             }
                                                             else if ( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 5)
                                                             {
                                                                 checkSum += 7;
                                                             }
                                                         }
                                                         else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                                         {
                                                             checkSum += 6;
                                                         }
                                                     }
                                                     else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                                     {
                                                         checkSum += 2;
                                                     }
                                                     else
                                                     {
                                                    	 badFormed++;
                                                         checkSum += 1;
                                                     }
                                                 }
                                             }
                                             else//11 axles
                                             {
                                                 if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                                 {
                                                	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
														 badFormed++;
													 }
                                                     checkSum += 1;
                                                     if ((((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                                     {
                                                         checkSum += 3;
                                                         if ( (auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                                         {
                                                             checkSum += 4;
                                                             if ( (auxAxlesData[8].distance > fourPositionRange || auxAxlesData[8].distance == 0) && auxAxlesData[8].speed > 5)
                                                             {
                                                                 checkSum += 5;//13
                                                                 /**************IF IS ON LIMIT WE COULDN큈 SAFE***********/
                                                                 /*if ( (auxAxlesData[10].distance > distanceRange || auxAxlesData[10].distance == 0) && auxAxlesData[10].speed > 5)
                                                                 {
                                                                     checkSum += 19;//32
                                                                 }*/
                                                             }
                                                             else if ( (auxAxlesData[9].distance > distanceRange || auxAxlesData[9].distance == 0) && auxAxlesData[9].speed > 5)
                                                             {
                                                                 checkSum += 9;
                                                             }
                                                         }
                                                         else if ( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 5)
                                                         {
                                                             checkSum += 7;
                                                         }
                                                     }
                                                     else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                                     {
                                                         checkSum += 6;
                                                     }
                                                 }
                                                 else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                                 {
                                                     checkSum += 2;
                                                 }
                                                 else
                                                 {
													 badFormed++;
                                                     checkSum += 1;
                                                 }
                                             }
                                         }
                                         else//10 axles
                                         {
                                             if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                             {
                                            	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
													 badFormed++;
												 }
                                                 checkSum += 1;
                                                 if ( (((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                                 {
                                                     checkSum += 3;
                                                     if ( (auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                                     {
                                                         checkSum += 4;
                                                         if ( (auxAxlesData[8].distance > fourPositionRange || auxAxlesData[8].distance == 0) && auxAxlesData[8].speed > 5)
                                                         {
                                                             checkSum += 5;
                                                         }
                                                         else if( (auxAxlesData[9].distance > distanceRange || auxAxlesData[9].distance == 0) && auxAxlesData[9].speed > 5)
                                                         {
                                                             checkSum += 9;
                                                         }
                                                     }
                                                     else if ( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 5)
                                                     {
                                                         checkSum += 7;
                                                     }
                                                 }
                                                 else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                                 {
                                                     checkSum += 6;
                                                 }
                                             }
                                             else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                             {
                                                 checkSum += 2;
                                             }
                                             else
                                             {
                                            	 badFormed++;
                                                 checkSum += 1;
                                             }
                                         }
                                     }
                                     else//9 axles
                                     {
                                         if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                         {
                                        	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
												 badFormed++;
											 }
                                             checkSum += 1;
                                             if ( (((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                             {
                                                 checkSum += 3;
                                                 if ( (auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                                 {
                                                     checkSum += 4;
                                                     /**************IF IS ON LIMIT WE COULDN큈 SAFE***********/
                                                     /*if ( (auxAxlesData[8].distance > distanceRange || auxAxlesData[8].distance == 0) && auxAxlesData[8].speed > 5)
                                                     {
                                                         checkSum += 15;
                                                     }*/
                                                 }
                                                 else if ( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 5)
                                                 {
                                                     checkSum += 7;
                                                 }
                                             }
                                             else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                             {
                                                 checkSum += 6;
                                             }
                                         }
                                         else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                         {
                                             checkSum += 2;
                                         }
                                         else
                                         {
                                        	 badFormed++;
                                             checkSum += 1;
                                         }
                                     }
                                 }
                                 else//8 axles
                                 {
                                     if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                     {
                                    	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
											 badFormed++;
										 }
                                         checkSum += 1;
                                         if ( (((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                         {
                                             checkSum += 3;
                                             if ( (auxAxlesData[6].distance > fourPositionRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                             {
                                                 checkSum += 4;
                                             }
                                             else if( (auxAxlesData[7].distance > distanceRange || auxAxlesData[7].distance == 0) && auxAxlesData[7].speed > 5)
                                             {
                                                 checkSum += 7;
                                             }
                                         }
                                         else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                         {
                                             checkSum += 6;
                                         }
                                     }
                                     else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                     {
                                         checkSum += 2;
                                     }
                                     else
                                     {
                                    	 badFormed++;
                                         checkSum += 1;
                                     }
                                 }
                             }
                             else//7 axles
                             {
                                 if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                                 {
                                	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
										 badFormed++;
									 }
                                     checkSum += 1;
                                     if ((((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                     {
                                         checkSum += 3;
                                         /**************IF IS ON LIMIT WE COULDN큈 SAFE***********/
                                         /*if ( (auxAxlesData[6].distance > distanceRange || auxAxlesData[6].distance == 0) && auxAxlesData[6].speed > 5)
                                         {
                                             checkSum += 13;
                                         }*/
                                     }
                                     else if ( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                     {
                                         checkSum += 6;
                                     }
                                 }
                                 else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                                 {
                                     checkSum += 2;
                                 }
                                 else
                                 {
                                	 badFormed++;
                                     checkSum += 1;
                                 }
                             }
                         }
                         else//6 axles
                         {
                             if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                             {
                            	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
									 badFormed++;
								 }
                                 checkSum += 1;
                                 if ((((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                                 {
                                     checkSum += 3;
                                 }
                                 else if( (auxAxlesData[5].distance > distanceRange || auxAxlesData[5].distance == 0) && auxAxlesData[5].speed > 5)
                                 {
                                     checkSum += 6;
                                 }
                             }
                             else if ( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                             {
                                 checkSum += 2;
                             }
                             else
                             {
                            	 badFormed++;
                                 checkSum += 1;
                             }
                         }
                     }
                     else//5 axles
                     {
                         if ( (auxAxlesData[2].distance > distanceRange || auxAxlesData[2].distance == 0) && auxAxlesData[2].speed > 5)
                         {
                        	 if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
								 badFormed++;
							 }
                             checkSum += 1;
                             /**************IF IS ON LIMIT WE COULDN큈 SAFE***********/
                             /*if ( (((auxAxlesData[4].distance > distanceRange) && (auxAxlesData[4].distance > fourPositionRange)) || auxAxlesData[4].distance == 0) && auxAxlesData[4].speed > 5)
                             {
                                 checkSum += 10;
                             }*/
                         }
                         else if( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                         {
                             checkSum += 11;
                         }
                         else
                         {
                        	 badFormed++;
                             checkSum += 1;
                         }
                     }
                 }
                 else if(lastAxles == 4)//4 axles
                 {
                	 //JUST A LOCOMOTIVE AXLES
                     if( (auxAxlesData[3].distance > distanceRange || auxAxlesData[3].distance == 0) && auxAxlesData[3].speed > 5)
                     {
                         return qtyAxles - axlesIt;
                     }

                     if(badFormed > 0 && auxAxlesData[0].distance > fourPositionRange){
						 badFormed++;
					 }
					 gettingCars = false;
					 checkSum = 1;
                 }
                 else
                 {
                     return qtyAxles - axlesIt;
                 }
             }//else 14

			 if(badFormed > 3){
				 return -1;
			 }

			 if (checkSum == 0)
			 {
				 carAxles = qtyAxles - axlesIt;
			 }else if (checkSum == 1){
				 carAxles = 4;
			 }else if (checkSum == 2 || checkSum == 4){
				 carAxles = 6;
			 }else if (checkSum == 7){
				 carAxles = 3;
			 }else if (checkSum == 8){
				 carAxles = 8;
			 }else if (checkSum == 9 || checkSum == 11){
				 carAxles = 5;
			 }else if (checkSum == 13){
				 carAxles = 10;
			 }else if (checkSum == 17){
				 carAxles = 7;
			 }else if (checkSum == 21){
				 carAxles = 12;
			 }else if (checkSum == 23){
				 carAxles = 9;
			 }else if (checkSum == 32){
				 carAxles = 11;
			 }else{
				 return -1;
			 }

			 int auxAxlesIt = carAxles + axlesIt;
			 /**********************************WHAT TO DO WHEN CAR IS FORMED******************************************/
			 int auxProfIt = profileRREQty;
			 memcpy(&profileRRE[auxProfIt], &emptyRRE, sizeof(typeT94Rre));
			 convIntToAscii(carAxles, profileRRE[auxProfIt].axlesCount, 3);

			 //convIntToAscii((blockAxles[axlesIt].speed / 10), profileRRE[auxProfIt].speedVehicle, 3);
			 int speedSum = 0;
			 int speedProm = 0;
			 for(int x = axlesIt; x < auxAxlesIt; x++){
				 speedSum += blockAxles[x].speed;
			 }
			 speedProm = speedSum / carAxles;
			 convIntToAscii( (speedProm / 10), profileRRE[auxProfIt].speedVehicle, 3 );

			 if(blockAxles[axlesIt].dir > 0){
				 convIntToAscii(blockAxles[axlesIt].dir, profileRRE[auxProfIt].direction, 1);
			 }

			 //FOR DETECT IF IS A CAR OR A LOCOMOTIVE
			 if(carAxles == 6 || carAxles ==5){
				 if(blockAxles[axlesIt + 3].distance > distanceRange){
					 if(blockAxles[axlesIt + 3].distance > 500 && blockAxles[axlesIt + 3].distance < 800 && carAxles == 6){
						 profileRRE[auxProfIt].groupCode[0] = 'D';
						 convIntToAscii(1, profileRRE[auxProfIt].platformCount, 3);
					 }else{
						 profileRRE[auxProfIt].groupCode[0] = 'R';
						 convIntToAscii(2, profileRRE[auxProfIt].platformCount, 3);
					 }
				 }else if(blockAxles[axlesIt + 2].distance > distanceRange){
					 profileRRE[auxProfIt].groupCode[0] = 'R';
					 convIntToAscii(2, profileRRE[auxProfIt].platformCount, 3);
				 }
			 }else{
				 if(blockAxles[axlesIt + 2].distance > distanceRange){
					 profileRRE[auxProfIt].groupCode[0] = 'R';
					 if(carAxles % 2 == 0){
						 convIntToAscii((carAxles - 2) / 2, profileRRE[auxProfIt].platformCount, 3);
					 }else{
						 convIntToAscii((carAxles - 1) / 2, profileRRE[auxProfIt].platformCount, 3);
					 }
				 }else if(carAxles == 4 || carAxles == 3){
					 if( blockAxles[axlesIt + 2].distance > 245 && blockAxles[axlesIt + 2].distance < 350 ){
						 profileRRE[auxProfIt].groupCode[0] = 'D';
					 }else{
						 profileRRE[auxProfIt].groupCode[0] = 'R';
					 }
					 convIntToAscii(1, profileRRE[auxProfIt].platformCount, 3);
				 }
			 }

			 copyAeiTime(&profileRRE[auxProfIt].startTime, &blockAxles[axlesIt].time);
			 copyAeiTime(&profileRRE[auxProfIt].endTime, &blockAxles[auxAxlesIt-1].time);

			 profileRREQty++;

			 convIntToAscii(profileRREQty, profileRRE[auxProfIt].sequenceNumber, 3);

			 axlesIt = auxAxlesIt;
		 }//while
		 return 0;
	}else{
		return qtyAxles;
	}
	return 0;
}

typeAxle blockAxles[MAX_AXLES_TRAIN];
/**********************************EXTRACT THE BLOCK DIRECTIONS AND TO MAKE THE RRES******************************/
bool trainAxlesProfile(){
	if(testMode){
		return false;
	}

	profileRREQty = 0;
	int axleIt = 0;
	int axlesBlockQty = 0;
	int prevDir = 0;

	while(axleIt < trainData->qtyAxles){
		typeAxle *axle = &trainData->axlesData[axleIt];
		if(prevDir == 0){
			prevDir = axle->dir;
			memcpy(&blockAxles[axlesBlockQty], axle, sizeof(typeAxle));
			axlesBlockQty++;
		}else{
			if(prevDir == axle->dir){
				memcpy(&blockAxles[axlesBlockQty], axle, sizeof(typeAxle));
				axlesBlockQty++;
			}else{
				/*TO PROCESS THE AXLES BLOCK TO MAKE CARS*/
				int residual = processBlockToRRE(blockAxles, axlesBlockQty);
				if(residual > 0){
					/**************TO DETERMINATE HOW MANY AXLES MUST BE DELETED FROM AXLES LIST*************************/
					int auxDir = axle->dir;
					int axCount = 0;
					for(int itAux = axleIt; itAux < trainData->qtyAxles && axCount < residual; itAux++){
						if(auxDir == trainData->axlesData[itAux].dir){
							axCount++;
						}else{
							if(axCount < residual){
								residual += axCount;
							}
							itAux = trainData->qtyAxles;
						}
					}
					axleIt--;
					axleIt += residual;
					axlesBlockQty = 0;
				}else if(residual == 0){
					axlesBlockQty = 0;
					memcpy(&blockAxles[axlesBlockQty], axle, sizeof(typeAxle));
					axlesBlockQty++;
				}else{
					profileRREQty = 0;
					return false;
				}
				prevDir = axle->dir;
			}
		}
		axleIt++;
	}
	if(axlesBlockQty > 0){
		int residual = processBlockToRRE(blockAxles, axlesBlockQty);
		if(residual < 0){
			profileRREQty = 0;
			return false;
		}
	}
	return true;
}

/**********************************EXTRACT THE AXLES STORED IN FIFO AND SAVE IT INTO AXLES STRUCT ARRAY******************************/
bool axlesDebugging(){
	if(!trainData)
		return false;

	bool result = true;

	if(trainData->qtyAxles <= 4)
		result = false;

	int i;
	const int axDiffRange = 30000;
	const int axDistRange = 1600;
	typeAxle * lastaAxle = NULL;
	logg.newPrint("\n****************Printing %d axles***************", trainData->qtyAxles);
	typeAxle * axle;
	for(i = 0; i < MAX_AXLES_TRAIN && i < trainData->qtyAxles; i++){

		axle = &trainData->axlesData[i];
		printAxle(axle, i + 1);

		if(lastaAxle){
			int diff = difAeiTime(&lastaAxle->time, &axle->time);
			int speedDiff = lastaAxle->speed - axle->speed;
			if(speedDiff < 0)
				speedDiff *= -1;
			if( (diff >= axDiffRange && axle->distance >= axDistRange) || (speedDiff > 5 && axle->distance >= axDistRange) || (diff >= axDiffRange && speedDiff > 5 && axle->distance > 0)){
				result = false;
				logg.newPrint("		; BAD AXLE");
			}else{
				if(trainData->qtyAxles == 4 && axle->distance >= 1500){
					result = false;
					logg.newPrint("		; BAD AXLE");
				}
			}
		}
		lastaAxle = axle;
	}
	logg.newPrint("\n****************Finish print axles***************");

	return result;
}

/**********************************DECODE THE AARTAGS TO RRES AND SAVE IT INTO RRES TAGS ARRAY******************************/
void decEncAarToRre(Fifo *aarTags){
	typeAarDec * newTagAar;
	typeT94Rre * newCar = NULL;
	int seqNumber = 1;
	int axDecByTags = 0;
	tagsRREQty = 0;

	logg.newPrint("\nStarting process %d Aar tags decoded from Hex Tags saved",  aarTags->FifoSize());

	while(aarTags->FifoSize() > 0){
		newTagAar = (typeAarDec *)aarTags->Get();
		if(newTagAar != NULL){
			printAarTag(newTagAar);
			if(newTagAar->groupCode == eqGrpCodeEot){
				if(seqNumber > 1){
					seqNumber--;//THE SECUENCE OF EOT MUST BE THE SAME THAT LAST CAR, BECAUSE IT IS MOUNTED ON THAT
				}
				fillEot(newTagAar, &seqNumber);
			}else{
				newCar = makeCars(newTagAar, &seqNumber, FALSE);
				if(newCar != NULL){
					axDecByTags += newCar->axlesCountTag;
					printRreCar(newCar);
					memcpy(&tagsRRE[tagsRREQty++], newCar, sizeof(typeT94Rre));
				}
			}
		}
		aarTags->MoveReader(TRUE);
	}

	newCar = makeCars(NULL, &seqNumber, TRUE);

	if(newCar != NULL){
		axDecByTags += newCar->axlesCountTag;
		printRreCar(newCar);
		//addRreToTrain(newCar);
		memcpy(&tagsRRE[tagsRREQty++], newCar, sizeof(typeT94Rre));
	}
	qtyAxlesTag = axDecByTags;
	logg.newPrint("\nFinished process Aar Tags decoded");
}

void decHexToAar(){
	aarTags.Initiate();
	logg.newPrint("\n\nStarting process %d Hex Tags saved",  trainData->qtyHexTags);
	initIncAarTag();
	for (int itHex = 0; itHex < trainData->qtyHexTags; itHex++) {
		typeAarDec * aarTag = decEncHexToAar( &trainData->hexTagData[itHex] );
		if(aarTag){
			aarTags.Add(aarTag);
		}
	}
	logg.newPrint("\nFinished process Hex Tags saved");
}

bool profileWork = FALSE;
/*************THE return 0 say to the train process that get the AXLESQTY from the RRE added*
 * return > 0 say that profiler could not work so use my axles detected
 * ************/
int getTotalAxles(){
	if(profileWork){
		return 0;
	}else{
		return trainData->qtyAxles;
	}
}

bool createTrain(){
	profileWork = FALSE;

	//if there is not axles
	if(trainData->qtyAxles <= 0)
		return false;

	/**************CALL TO TRAIN TO CLEAN THE BUFFERS***********************/
	//initTrain();
	newTrainProcess();

	decHexToAar();

	decEncAarToRre(&aarTags);

	profileWork = axlesDebugging();

	if(profileWork){
		logg.newPrint("\nMAKING PROFILER");
		profileWork = trainAxlesProfile();
	}

	if(profileWork){
		logg.newPrint("\nRRE FORMED BY PROFILER, MATCHING AXLES' RRES WITH TAGS' RRES");
		matchRREs();
	}else{
		logg.newPrint("\nRRE PROFILER COULD NOT FORM THE TRAIN SETTING THE RRES TAGS");
		setToTrainRRETags();//UNCOMMENT WHEN RRES IS FORMED WITH AXLES
	}

	qtyCars = getTrainRreQty();

	if(qtyCars == 0)
		return false;

	return true;
}

SDTrainUsers trainDBUserW;
void saveTrainIntoSD(){
	/*******************SAVE train into sd********************/
	typeTrain * train = getTrain();
	int trainNum = 1;
	trainDBUserW.request = WriteRequest;
	trainDBUserW.dataType = TrainData;
	trainDBUserW.modReqStFlag = true;
	trainDBUserW.useTrainControl = true;
	while(train){
		//MAKE THE REQUEST TO DB TRAIN
		train->reportedFlag = false;
		memcpy(train->packetId, trainData->packetId, 13);
		trainDBUserW.buffSize = CalcTrainSize(train);
		trainDBUserW.dataBuffer = train;
		trainDBUserW.stateRequest = StNoRequest;
		while(!newRequest(&trainDBUserW)){
			OSTimeDly(1);
		}
		OSTimeDly(1);
		//WILL WAIT UNTIL THE REQUEST HAS BEEN TERMINATED
		while(trainDBUserW.stateRequest == StInProgressReq){
			OSTimeDly(1);
		}
		if(trainDBUserW.stateRequest == StCompletedReq){
			logg.newPrint("\nTrain %d Stored in SD Card. PacketId: %s", trainNum++, train->packetId);
		}else{
			logg.newPrint("\nTrain %d could not be stored. PacketId: %s \nCheck SD Card", trainNum++, train->packetId);
		}

		freeTrain(train);
		train = getTrain();
	}
}

enum ProcTrainState { StInit, StWating, StMakeRequest, StWaitRequest, StProcTrain, StSaveProcTrain, StAfterProcTrain};

SDTrainUsers trainDBUserRLog;
ProcTrainState ProcState = StInit;
int ticksToReq = 0;
void makeDataRequest(RequestType request){
	trainDBUserRLog.request = request;
	trainDBUserRLog.dataType = TrainLogData;
	trainDBUserRLog.modReqStFlag = true;
	trainDBUserRLog.useTrainControl = true;
	if(request == ReadRequest){
		trainData = (typeTrainData *)malloc(sizeof (typeTrainData));
		trainDBUserRLog.buffSize = sizeof (typeTrainData);
	}else{
		trainData->processedFlag = true;
		trainDBUserRLog.buffSize = CalcTrainDataSize(trainData);
	}
	trainDBUserRLog.dataBuffer = trainData;
	trainDBUserRLog.stateRequest = StInProgressReq;
	while(!newRequest(&trainDBUserRLog)){
		OSTimeDly(1);
	}
}


void getDataTrain(){
	switch(ProcState){
		case StInit:
			ticksToReq = 0;
			ProcState = StWating;
			break;
		case StWating:
			if(ticksToReq++ >= 30){
				ProcState = StMakeRequest;
				ticksToReq = 0;
			}
			break;
		case StMakeRequest:{
				makeDataRequest(ReadRequest);
				ProcState = StWaitRequest;
			}
			break;
		case StWaitRequest:
			if(trainDBUserRLog.stateRequest != StInProgressReq){
				if(trainDBUserRLog.request == ReadRequest){
					if(trainDBUserRLog.stateRequest == StCompletedReq){
						ProcState = StProcTrain;
					}else{
						free(trainData);
						trainData = NULL;
						ProcState = StWating;
					}
				}else{
					free(trainData);
					trainData = NULL;
					ProcState = StWating;
				}
			}
			break;
		case StProcTrain:
			if(createTrain()){
				ProcState = StSaveProcTrain;
			}else{
				ProcState = StAfterProcTrain;
			}
			break;
		case StSaveProcTrain:
			saveTrainIntoSD();
			ProcState = StAfterProcTrain;
			break;
		case StAfterProcTrain:
			makeDataRequest(OverWriteRequest);
			ProcState = StWaitRequest;
			break;
	}
}

SDTrainUsers trainDBUserWLog;
/********************************************MAKE A STRUCTURE WITH ALL AXLES AND HEXTAGS TO BE STORE IN SD CARD*****************/
void fillDataTrain(Fifo * hexTags){
	if(axles.FifoSize() <= 0){
		return;
	}

	typeTrainData * trainData = (typeTrainData *)malloc(sizeof (typeTrainData));
	trainData->processedFlag = FALSE;
	trainData->qtyAxles = 0;
	trainData->qtyHexTags = 0;

	/************************************FOR PACKAGE IDENTIFIER**********************************/
	TrainControl * myControl = getControlTrain(TrainLogData);
	memset(trainData->packetId, '\0', 13);
	memcpy(trainData->packetId, myControl->DayDirWrite, 9);
	sprintf(trainData->packetId + 8, "%04d", (int)myControl->TrainNumWrite + 1);
	trainData->packetId[12] = '\0';

	while(hexTags->FifoSize() > 0) {
		typeHexTag * newTag = (typeHexTag *)hexTags->Get();
		memcpy(&trainData->hexTagData[trainData->qtyHexTags++], newTag, sizeof(typeHexTag));
		hexTags->MoveReader(TRUE);
	}

	while(axles.FifoSize() > 0){
		typeAxle * axle = (typeAxle *) axles.Get();
		memcpy(&trainData->axlesData[trainData->qtyAxles++], axle, sizeof(typeAxle));
		axles.MoveReader(TRUE);
	}
	axles.Initiate();

	/*TO SET TRAIN DATA TO SD*/
	trainDBUserWLog.request = WriteRequest;
	trainDBUserWLog.dataType = TrainLogData;
	trainDBUserWLog.modReqStFlag = true;
	trainDBUserWLog.useTrainControl = true;
	trainDBUserWLog.buffSize = CalcTrainDataSize(trainData);
	trainDBUserWLog.dataBuffer = trainData;
	trainDBUserWLog.stateRequest = StNoRequest;

	while(!newRequest(&trainDBUserWLog)){
		OSTimeDly(1);
	}
	OSTimeDly(1);
	//WILL WAIT UNTIL THE REQUEST HAS BEEN TERMINATED
	while(trainDBUserWLog.stateRequest == StInProgressReq){
		OSTimeDly(1);
	}
	if(trainDBUserWLog.stateRequest == StCompletedReq){
		logg.newPrint("\nTrain Package %s Stored in SD Card", trainData->packetId);
	}else{
		logg.newPrint("\nTrain Package %s could not be stored \nCheck SD Card", trainData->packetId);
	}

	free(trainData);
	trainData = NULL;
}
//Protect against zero axles or zero cars

