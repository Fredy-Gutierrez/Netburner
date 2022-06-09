/*
 * tagUtilities.cpp
 *
 *  Created on: 12 abr 2021
 *      Author: Integra Fredy
 */
#include <nbstring.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <libs/TagUtilities.h>
#include "libs/Rtc_Controller.h"
#include "libs/Log.h"

extern logger logg;

#define SIZE_HEX_HEXTAG			30
#define SIZE_AUX_DATA_HEXTAG	9
#define INDEX_AUX_DATA_HEXTAG	54

void hexToBit(char input[], char * res){
	const char binary[16][5] = {"0000", "0001", "0010", "0011", "0100","0101","0110","0111","1000","1001",
								"1010","1011","1100","1101","1110","1111"};

	const char digits[] = "0123456789abcdef";

	res[0] = '\0';
	int p = 0;

	while(input[p])
	{
	    const char *v = strchr(digits, tolower(input[p++]));
	    if (v)
	        strcat(res, binary[v - digits]);
	}
}

long long int binaryToDecimal(char binaryInput[])
{
	long long int decimal = 0;
	int position = 0;
	for(int i = strlen(binaryInput)-1; i >= 0; --i){
		if(binaryInput[i]=='1'){
			decimal += (long long int)(pow(2,position));
		}
		++position;
	}

	return decimal;
}

/*******************************************PROCESSOR INIT*************************************************/
char * getInitials(int val){
	char *str = "    ";
	str[0]='\0';
	int n1, n2, n3, n4;

	n1 = val/BASEe3;
	strncat(str, &C1[n1], 1);

	n2 = (val - (n1 * BASEe3))/BASEe2;
	strncat(str, &C2_4[n2], 1);

	n3 = (val - ((n1 * BASEe3) + (n2 * BASEe2)))/BASE;
	strncat(str, &C2_4[n3], 1);

	n4 =  val - ((n1 * BASEe3) + (n2 * BASEe2) + (n3 * BASE));
	strncat(str, &C2_4[n4], 1);

	str[4]='\0';

	return str;
}

void setBinSection(char * binSection, char * bin, int idxIni, int idxEnd){
	memset(binSection,'\0',23);
	for(int j=0, i=idxIni; i<=idxEnd; i++, j++){
		binSection[j] = bin[i];
	}
}


bool processBin(char bin[], typeAarDec *tag){
	char binSection[23];

	// Data format code	 		 6 bits 114 to 119			(118 to 123)
	setBinSection(binSection, bin, 114, 119);
	tag->dataFormatCode = int(binaryToDecimal(binSection));

	// Equipment group code 	 5 bits  0 to  4
	setBinSection(binSection, bin, 0, 4);
	tag->groupCode = int(binaryToDecimal(binSection));

	// Tag type 				 2 bits  5 to  6
	setBinSection(binSection, bin, 5, 6);
	tag->tagType = int(binaryToDecimal(binSection)) + 1;

	//data format code identifies the coding of the tag, there are two codes that have been received in the AEIs of Ferrovalle and Api
	//100101 (37d) used for locomotive dynamic tags
	//110011 (51d) used for other tags, not dynamic tags
	//Coding of both (37 and 51), have some fields that are decoded equal, these are:
	//Equipment group code
	//Tag type
	//Equipment initial mark
	//Car number
	//Side indicator code
	//Length
	//Number of axles
	//Bearing type code

	// Equipment initial mark	19 bits  7 to 25
	setBinSection(binSection, bin, 7, 25);
	strcpy(tag->equipmentInitial, getInitials(int(binaryToDecimal(binSection))));

	// Car number				20 bits 26 to 45
	setBinSection(binSection, bin, 26, 45);
	tag->carNumber = int(binaryToDecimal(binSection));

	// Number of axles			 5 bits 56 to 60			(64)
	setBinSection(binSection, bin, 56, 60);
	tag->axlesNumber = int(binaryToDecimal(binSection)) + 1;

	// Bearing type code		 3 bits 61 to 63			(65 to 67)
	setBinSection(binSection, bin, 61, 63);
	tag->bearingCode = int(binaryToDecimal(binSection));

	tag->reserved=0;
	tag->frameNumber=0;

	switch(tag->dataFormatCode){
		case fmtCodeNonDyn:
			if(tag->tagType==typTagNonDyn || tag->tagType==typTagReserved){
				switch(tag->groupCode){
					case eqGrpCodeRailcar:
						// Side indicator code		 1 bit  46
						setBinSection(binSection, bin, 46, 46);
						tag->sideIndicator = int(binaryToDecimal(binSection));

						// Length					12 bits 90 to 92, 47 to 55 	for group code == 19		(94 to 96)
						setBinSection(binSection, bin, 90, 92);
						for(int j=3, i=47; i<=55; i++, j++){
							binSection[j] = bin[i];
						}
						tag->decimetersLength = int(binaryToDecimal(binSection));

						// Platform identifier code	 4 bits 64 to 67			(68 to 71)
						setBinSection(binSection, bin, 64, 67);
						tag->platformCode = int(binaryToDecimal(binSection));

						tag->eotType=0;
					break;

					case eqGrpCodeLocomotive:
						// Side indicator code		 1 bit  46
						setBinSection(binSection, bin, 46, 46);
						tag->sideIndicator = int(binaryToDecimal(binSection));

						// Length					 9 bits 47 to 55 			for group code == 5
						setBinSection(binSection, bin, 47, 55);
						tag->decimetersLength = int(binaryToDecimal(binSection));

						tag->platformCode=0;
						tag->eotType=0;
					break;

					case eqGrpCodeEot:
						// Side indicator code		 1 bit  48
						setBinSection(binSection, bin, 48, 48);
						tag->sideIndicator = int(binaryToDecimal(binSection));

						// EOT type 				 2 bits  46 to  47
						setBinSection(binSection, bin, 46, 47);
						tag->eotType = int(binaryToDecimal(binSection));


						tag->platformCode=0;
						tag->axlesNumber=0;
						tag->bearingCode=0;
					break;

					default:
						return false;
				}
			}
			else{
				return false;
			}
		break;

		case fmtCodeDyn:
			if(tag->tagType==typTagMultiFrag){
				// Side indicator code		 1 bit  100				(104)
				setBinSection(binSection, bin, 100, 100);
				tag->frameNumber = int(binaryToDecimal(binSection));

				if(tag->frameNumber==0){
					// Side indicator code		 1 bit  46
					setBinSection(binSection, bin, 46, 46);
					tag->sideIndicator = int(binaryToDecimal(binSection));

					// Length					 9 bits 47 to 55 			for group code == 5
					setBinSection(binSection, bin, 47, 55);
					tag->decimetersLength = int(binaryToDecimal(binSection));

					// Alarm codes					4 bits 64 to 67			(68 to 71)
					setBinSection(binSection, bin, 64, 67);
					tag->alarmCodes = int(binaryToDecimal(binSection));

					// Volume of fuel tank			6 bits 68 to 73			(72 to 77)
					setBinSection(binSection, bin, 68, 73);
					tag->volFuelTank = int(binaryToDecimal(binSection));

					// Cumulative kw hours			14 bits 74 to 87		(78 to 91)
					setBinSection(binSection, bin, 74, 87);
					tag->cumKwHr = int(binaryToDecimal(binSection));

					// Six dynamic parameters		each parameter has 2 bits, from 88-89 to 98-99		(92-93 to 102-103)
					for(int j=88, k=0; k<(qtyDylPar-1); j+=2, k++){
						setBinSection(binSection, bin, j, j);
						tag->dylPar[k].par = int(binaryToDecimal(binSection));
						setBinSection(binSection, bin, j+1, j+1);
						tag->dylPar[k].rep = int(binaryToDecimal(binSection));
					}
					// One dynamic parameters		2 bits, 110 to 111		(114 to 115)
					setBinSection(binSection, bin, 110, 110);
					tag->dylPar[qtyDylPar-1].par = int(binaryToDecimal(binSection));
					setBinSection(binSection, bin, 111, 111);
					tag->dylPar[qtyDylPar-1].rep = int(binaryToDecimal(binSection));

					// Communication status indicator	1 bit 101			(105)
					setBinSection(binSection, bin, 101, 101);
					tag->commsStatus = int(binaryToDecimal(binSection));

					// ETS switch cumulative count	4 bits 104 to 107	(108 to 111)
					setBinSection(binSection, bin, 104, 107);
					tag->etsSwitch = int(binaryToDecimal(binSection));

					tag->platformCode=0;
					tag->eotType=0;
				}
				else{
					return false;
				}
			}
			else{
				return false;
			}
		break;

		default:
			return false;
	}
	return true;
}

int getDecFrmCharHex(char c){
	int val;
	val=(int)c -0x30;
	if(val>9)
		val=val-7;
	return val;
}

int getDecFrmChar(char c){
	return (int)c -0x30;
}

void processAuxData(char auxData[], typeAarDec *tag){
	tag->antennaNumber=getDecFrmChar(auxData[0]);
	tag->timesRead=getDecFrmCharHex(auxData[3])+getDecFrmCharHex(auxData[2])*16;
	tag->rssi=(getDecFrmChar(auxData[8])+getDecFrmChar(auxData[7])*10+getDecFrmChar(auxData[6])*100)*(-1);
}

// Returns TRUE if is has only '0'
bool getHexMsg(const char * tag, char *hexTag, char *auxData){
	bool tagZeroed=TRUE;
	for(int i=0, j=1; i<SIZE_HEX_HEXTAG; i++, j++){
		hexTag[i]=tag[j];
		if(tag[j]!='0')
			tagZeroed=FALSE;
	}
	for(int i=0, j=INDEX_AUX_DATA_HEXTAG; i<SIZE_AUX_DATA_HEXTAG; i++, j++){
		auxData[i]=tag[j];
	}
	hexTag[SIZE_HEX_HEXTAG] = '\0';
	auxData[SIZE_AUX_DATA_HEXTAG] = '\0';

	return tagZeroed;
}

/* This function compares two tags decoded in AAR format                   */
/* Comparation excludes time, times read and RSSI                          */
bool cmpTag(typeAarDec * tag1, typeAarDec * tag2){
	if(tag1->groupCode==tag2->groupCode && tag1->tagType==tag2->tagType && tag1->carNumber==tag2->carNumber &&
		tag1->sideIndicator==tag2->sideIndicator && tag1->decimetersLength==tag2->decimetersLength &&
		tag1->axlesNumber==tag2->axlesNumber && tag1->bearingCode==tag2->bearingCode &&	tag1->platformCode==tag2->platformCode &&
		/*tag1->spare == tag2->spare && tag1->reserved == tag2->reserved && tag1->security==tag2->security &&*/
		tag1->dataFormatCode==tag2->dataFormatCode &&	tag1->antennaNumber==tag2->antennaNumber && tag1->direction==tag2->direction) {
		for(int i=0; i<5; i++){
			if(tag1->equipmentInitial[i] != tag2->equipmentInitial[i])
				return FALSE;
		}
		return TRUE;
	}
	else
		return FALSE;
}

void incSaveAuxData(typeAarDec * lastTag, typeAarDec * newTag){
	lastTag->timeLastRead.sysTime=newTag->timeFirstRead.sysTime;
	lastTag->timeLastRead.msCurrSec=newTag->timeFirstRead.msCurrSec;
	lastTag->timesRead+=newTag->timesRead;
	lastTag->rssi+=newTag->rssi;
	lastTag->speed+=newTag->speed;
}

bool tagReceived=FALSE;
typeAarDec lastTagRx;
int repeats=0;

void initIncAarTag(){
	tagReceived=FALSE;
	memset(& lastTagRx, 0, sizeof(typeAarDec));
	repeats=0;
}

/* This function receive a new tag decoded in AAR format                   */
/* If new tag is equal to last received, updates information regarding time, antenna reads and RSSI */
/* If new tag is different to last received, save new tag, and return an indication that last tag must be saved */
/* also returns pointer to last tag so it can be saved in the fifo */
bool updateIncAarTag(typeAarDec * rxTag, typeAeiTime * timeRx){
	/*static bool tagReceived=FALSE;
	static typeAarDec lastTagRx;
	typeAarDec tempTag;
	static int repeats=0;
	bool saveTag=FALSE;*/
	bool saveTag=FALSE;

	if(tagReceived){
		if(rxTag->reserved!=START_END_INDICATOR){
			//there is a tag received before in lastTagRx, check if it is equal to the one just received
			if(cmpTag(& lastTagRx, rxTag)){
				//tags are equal, updates time, increments read times, RSSI and repeats
				incSaveAuxData(& lastTagRx, rxTag);
				copyAeiTime(& lastTagRx.timeLastRead, timeRx);
				repeats++;
			}
			else{
				//tags are different, must save older tag in fifo, before calculates media value of RSSI
				lastTagRx.rssi/=repeats;
				lastTagRx.speed/=repeats;
				typeAarDec tempTag;
				memcpy(& tempTag, rxTag, sizeof(typeAarDec));
				memcpy(rxTag, & lastTagRx, sizeof(typeAarDec));
				memcpy(& lastTagRx, & tempTag, sizeof(typeAarDec));
				copyAeiTime(& lastTagRx.timeFirstRead, timeRx);
				copyAeiTime(& lastTagRx.timeLastRead, timeRx);
				repeats=1;
				saveTag=TRUE;
			}
		}
		else{
			//last tag, must save older tag in fifo, before calculates media value of RSSI
			lastTagRx.rssi/=repeats;
			lastTagRx.speed/=repeats;
			memcpy(rxTag, & lastTagRx, sizeof(typeAarDec));
			repeats=0;
			tagReceived=FALSE;//inits this var
			saveTag=TRUE;
		}
	}
	else{
		//saves tag received only if it is not startEndTag
		if(rxTag->reserved!=START_END_INDICATOR){
			memcpy(& lastTagRx, rxTag, sizeof(typeAarDec));
			copyAeiTime(& lastTagRx.timeFirstRead, timeRx);
			copyAeiTime(& lastTagRx.timeLastRead, timeRx);
			repeats=1;
			tagReceived=TRUE;
		}
	}
	return saveTag;
}

bool cmpAarTags(typeAarDec * tag, bool initialize){
	static char lastEquipmentInitial[5]="    ";
	static int lastCarNumber=-1;
	bool equal=TRUE;

	if(initialize){
		for(int i=0; i<5; i++)
			lastEquipmentInitial[i]=0;
		lastCarNumber=-1;
		return equal;
	}

	if(lastCarNumber!=tag->carNumber)
		equal=FALSE;
	else{
		for(int i=0; i<5; i++)
			if(lastEquipmentInitial[i]!=tag->equipmentInitial[i]){
				equal=FALSE;
				break;
			}
	}

	//update data
	for(int i=0; i<5; i++)
		lastEquipmentInitial[i]=tag->equipmentInitial[i];
	lastCarNumber=tag->carNumber;
	return equal;
}

void convIntToAscii(int value, char ascii[], int qtyDigits){
	for(int i = 0; i < qtyDigits;i++)
		ascii[i]='0';
	for(int number = value, digit, j = (qtyDigits - 1); j >= 0;){
		digit = (number % 10) + 0x30;
		number /= 10;
		ascii[j--] = (char)digit;
	}
}

typeT94Rre * fillSaveRreCar(typeAarDec * rTag, typeAarDec * lTag, typeTagStatus tagStatus, int * seqNumber){
	typeT94Rre * newCarRre;
	typeAarDec * refTag;
	bool orientation;
	bool bothSideTagsExist=TRUE;
	int antCount0, antCount1;
	int speed=0;

	//If both tags were read or only right one, use right tag to get all information that is equal in both tags
	if(tagStatus==bothTags){
		refTag=rTag;
		antCount1=rTag->timesRead;
		antCount0=lTag->timesRead;
		if(rTag->sideIndicator!=lTag->sideIndicator)
			bothSideTagsExist=TRUE;
		else
			bothSideTagsExist=FALSE;
	}
	else if(tagStatus==leftTagMissing){
		refTag=rTag;
		antCount1=rTag->timesRead;
		antCount0=0;
	}
	else{	//Only left tag was read, take left tag
		refTag=lTag;
		antCount1=0;
		antCount0=lTag->timesRead;
	}

	if(bothSideTagsExist){
		if(refTag->sideIndicator==refTag->antennaNumber)
			orientation=TRUE;
		else
			orientation=FALSE;
	}
	else
		orientation=TRUE;			//It is reported A, but really does not matter



/*	if(tagStatus==bothTags || tagStatus==leftTagMissing){
		refTag=rTag;
		if(rTag->sideIndicator==rTag->antennaNumber)
			orientation=TRUE;
		else
			orientation=FALSE;
	}
	else{	//Only left tag was read, take left tag
		refTag=lTag;
		if(lTag->sideIndicator==lTag->antennaNumber)
			orientation=TRUE;
		else
			orientation=FALSE;
	}
*/
	newCarRre = (typeT94Rre *)malloc(sizeof (typeT94Rre));

	newCarRre->segmentID[0]='R'; newCarRre->segmentID[1]='R'; newCarRre->segmentID[2]='E';
	convIntToAscii(* seqNumber, newCarRre->sequenceNumber, 3);
	//newCarRre->sequenceNumber[0]='0'; newCarRre->sequenceNumber[1]='0'; newCarRre->sequenceNumber[2]='1';
	switch(refTag->groupCode){
		case 5:
			newCarRre->groupCode[0]='D';
			if(orientation)
				newCarRre->orientation[0]='F';
			else
				newCarRre->orientation[0]='R';
			break;
		case 19:
			newCarRre->groupCode[0]='R';
			if(orientation)
				newCarRre->orientation[0]='A';
			else
				newCarRre->orientation[0]='B';
			break;
		case 6:
			newCarRre->groupCode[0]='T';
			break;
	}

	for(int i=0; i<4; i++)
		newCarRre->ownerCode[i]=refTag->equipmentInitial[i];
	convIntToAscii(refTag->carNumber, newCarRre->ownerEquipmentNumber, 10);
	newCarRre->carNumberTag=refTag->carNumber;

	newCarRre->reserved[0]=' ';
	// It must be complemented by axle processor
	newCarRre->axlesConversionCode[0]='G';
	newCarRre->tagStatus[0]=(char)tagStatus;
	// It must be complemented by axle processor
	if(bothSideTagsExist)
		newCarRre->tagDetailStatus[0]='K';
	else
		newCarRre->tagDetailStatus[0]='O';

	switch(tagStatus){
	case bothTags:
		if(cmpAeiTime(& rTag->timeFirstRead, & lTag->timeFirstRead)==1){
			copyAeiTime(& newCarRre->startTime, & lTag->timeFirstRead);
			copyAeiTime(& newCarRre->endTime, & rTag->timeLastRead);
		}
		else{
			copyAeiTime(& newCarRre->startTime, & rTag->timeFirstRead);
			copyAeiTime(& newCarRre->endTime, & lTag->timeLastRead);
		}
		speed=(int)(rTag->speed+lTag->speed)/2;
		break;

	case leftTagMissing:
		antCount1=rTag->timesRead;
		copyAeiTime(& newCarRre->startTime, & rTag->timeFirstRead);
		copyAeiTime(& newCarRre->endTime, & rTag->timeLastRead);
		speed=(int)rTag->speed;
		break;

	case rightTagMissing:
		copyAeiTime(& newCarRre->startTime, & lTag->timeFirstRead);
		copyAeiTime(& newCarRre->endTime, & lTag->timeLastRead);
		speed=(int)lTag->speed;
		break;

	case mismatchId:
	case noTagRead:
		break;
	}
	if(antCount1>99)
		antCount1=99;
	if(antCount0>99)
		antCount0=99;

	convIntToAscii(antCount0, newCarRre->antena0Count, 2);
	convIntToAscii(antCount1, newCarRre->antena1Count, 2);
	newCarRre->ant0Count=antCount0;
	newCarRre->ant1Count=antCount1;

	convIntToAscii(speed/10, newCarRre->speedVehicle, 3);
	newCarRre->speedTag=speed;

	// It must be complemented by axle processor
	convIntToAscii(refTag->axlesNumber, newCarRre->axlesCount, 3);
	newCarRre->axlesCountTag=refTag->axlesNumber;

	int platformCount=refTag->platformCode;
	if(platformCount==0) platformCount=1;
	convIntToAscii(platformCount, newCarRre->platformCount, 3);

	newCarRre->separator1[0]='&';
	if(refTag->direction==1)
		newCarRre->direction[0]='1';
	else
		newCarRre->direction[0]='0';
	convIntToAscii(refTag->decimetersLength, newCarRre->length, 4);

	newCarRre->tagBearingCode[0]=(char)refTag->bearingCode+0x30;
	newCarRre->tagPlatformCode[0]=(char)refTag->bearingCode+0x30;
	convIntToAscii(refTag->platformCode, newCarRre->tagPlatformCode, 2);

	newCarRre->separator2[0]='&';

	// It must be complemented by axle processor
	(* seqNumber)++;

	return newCarRre;
}

//A car is constructed by one or two tags, they are identified as tagRight and tagLeft
//tagRight is the one read by antenna 1 and left tag is read by antenna 0.
//notice that left and right tags in this consideration could have a different side indicator


typeT94Rre * makeCars(typeAarDec * tag, int * seqNumber, bool forceCarSave){

	static typeAarDec tagRight;
	static typeAarDec tagLeft;
	static typeTagStatus tagStatus=noTagRead;

	typeT94Rre * newCarRre;
	bool sameCar;
	if(!forceCarSave){
		sameCar=cmpAarTags(tag, FALSE);
		if(tagStatus==noTagRead){
			switch(tag->antennaNumber){
				case 1:				//Tag located at right side of the car facing forward
					memcpy(& tagRight, tag, sizeof(typeAarDec));
					tagStatus = leftTagMissing;
					break;
				case 0:				//Tag located at left side of the car facing forward
					memcpy(& tagLeft, tag, sizeof(typeAarDec));
					tagStatus = rightTagMissing;
					break;
			}
			return NULL;
		}
		else{	//One side tag has been identified
			if(sameCar){
				switch(tag->antennaNumber){
					case 1:				//Tag located at right side of the car facing forward
						memcpy(& tagRight, tag, sizeof(typeAarDec));
						if(tagStatus == rightTagMissing){
							tagStatus = bothTags;
						}
						break;
					case 0:				//Tag located at left side of the car facing forward
						memcpy(& tagLeft, tag, sizeof(typeAarDec));
						if(tagStatus == leftTagMissing){
							tagStatus = bothTags;
						}
						break;
				}
				//tagStatus = bothTags;
				newCarRre = fillSaveRreCar(&tagRight, &tagLeft, tagStatus, seqNumber);
				tagStatus=noTagRead;
				return newCarRre;
			}
			else{	//It was a different tag car, tag stored before must be used to make a car
				newCarRre = fillSaveRreCar(& tagRight, & tagLeft, tagStatus, seqNumber);
				if(tag->antennaNumber==1){
					memcpy(& tagRight, tag, sizeof(typeAarDec));
					tagStatus=leftTagMissing;
				}
				else{
					memcpy(& tagLeft, tag, sizeof(typeAarDec));
					tagStatus = rightTagMissing;
				}
				return newCarRre;
			}
		}
	}
	else{
		if(tagStatus != noTagRead){
			newCarRre = fillSaveRreCar(&tagRight, &tagLeft, tagStatus, seqNumber);
			cmpAarTags(tag, TRUE);	//just initialize equipment id and car number, for future comparing
			tagStatus = noTagRead;
			return newCarRre;
		}else{
			cmpAarTags(tag, TRUE);	//just initialize equipment id and car number, for future comparing
			tagStatus = noTagRead;
			return NULL;
		}
	}
}

void initStartEndTag(typeHexTag * sTag, typeHexTag * eTag){
	sTag->data[0]='*';
	sTag->data[1]='S';
	for(int i=2; i<(SIZE_HEX_HEXTAG-2); i++)
		sTag->data[i]=' ';
	sTag->data[SIZE_HEX_HEXTAG-2]='S';
	sTag->data[SIZE_HEX_HEXTAG-1]='*';
	for(int i=SIZE_HEX_HEXTAG; i<(SIZE_HEXTAG-1); i++)
		sTag->data[i]=' ';
	sTag->data[SIZE_HEXTAG-1]='\0';
	sTag->t.sysTime=0;
	sTag->t.msCurrSec=0;

	eTag->data[0]='*';
	eTag->data[1]='E';
	for(int i=2; i<(SIZE_HEX_HEXTAG-2); i++)
		eTag->data[i]=' ';
	eTag->data[SIZE_HEX_HEXTAG-2]='E';
	eTag->data[SIZE_HEX_HEXTAG-1]='*';
	for(int i=SIZE_HEX_HEXTAG; i<(SIZE_HEXTAG-1); i++)
		eTag->data[i]=' ';
	eTag->data[SIZE_HEXTAG-1]='\0';
	eTag->t.sysTime=0;
	eTag->t.msCurrSec=0;
}

bool cmpHexTags(typeHexTag * tag1, typeHexTag * tag2){
	for(int i=0; i<SIZE_HEXTAG; i++){
		if(tag1->data[i]!=tag2->data[i]){
			return false;
		}

	}
	if(tag1->t.sysTime==tag2->t.sysTime && tag1->t.msCurrSec==tag2->t.msCurrSec){
		return true;
	}
	else{
		return false;
	}
}

void printAarTag(typeAarDec *tagPrn){

	char asciiFormatIni[30]; char asciiFormatFin[30];
	convertTimeToASCII(& tagPrn->timeFirstRead, asciiFormatIni);
	convertTimeToASCII(& tagPrn->timeLastRead, asciiFormatFin);
	logg.newPrint("\nRailcar tag decoded data: TimeIni %s. TimeEnd: %s.   #%d.%d.%s.%d.%d.%d.%d.%d.%d.%d    AuxData:  %d.%d.%d.speed=%lu, dir=%d",
			asciiFormatIni, asciiFormatFin, tagPrn->groupCode, tagPrn->tagType,  tagPrn->equipmentInitial, tagPrn->carNumber,
			tagPrn->sideIndicator, tagPrn->decimetersLength, tagPrn->axlesNumber, tagPrn->bearingCode, tagPrn->platformCode,
			tagPrn->dataFormatCode,	tagPrn->antennaNumber, tagPrn->timesRead, tagPrn->rssi, tagPrn->speed, tagPrn->direction);
}

void printRreCar(typeT94Rre *carPrn){

	typeT94RreTimed * carSegment;
	char asciiFormatIni[30]; char asciiFormatFin[30];

	carSegment=(typeT94RreTimed *)carPrn;
	convertTimeToASCII(& carPrn->startTime, asciiFormatIni);
	convertTimeToASCII(& carPrn->endTime, asciiFormatFin);
	carPrn->separator2[0]='\0';
	logg.newPrint("\nRRE car decoded: %s  %s  %s", carSegment->segmentRre, asciiFormatIni, asciiFormatFin);
	carPrn->separator2[0]='&';

}

