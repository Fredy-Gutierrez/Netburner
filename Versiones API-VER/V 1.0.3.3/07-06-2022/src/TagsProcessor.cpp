/*
 * tagsProcessor.cpp
 *
 *  Created on: 5 abr 2021
 *      Author: Integra Fredy
 */
#include <nbrtos.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <libs/TagProcessor.h>
#include <libs/TagUtilities.h>
#include "libs/Rtc_Controller.h"
#include "libs/fifo/Fifo.h"
#include "libs/SargasProcess.h"
#include "libs/Report2Server.h"
#include "libs/Log.h"
#include "libs/AxleProcess.h"
#include "libs/Train.h"

/**********************************************OBJET TO SET AND GET THE TAG PROCESSED IN THE FIFO STRUCT**************************************************/
extern logger logg;

enum stTagProc {stProcIni, stProcWait, stProcDelayToCodeAar, stProcCodeAar, /*stProcCodeRre,*/ stProcTrain, stProcAxles, stReportToServer};
stTagProc stateTagProc;

Fifo hexTags;
Fifo aarTags;
typeHexTag startTag;
typeHexTag endTag;

bool foundEndTag = false, foundStartTag = false;//This will allow to just save tags before a ENDFOUND TAG

typeHexTag lastHexTagRx;
bool firstHexRaw;
int qtyRepTags;


void initLastHexRaw(){
	for(int i=0; i<SIZE_HEXTAG; i++){
		lastHexTagRx.data[i]=0;
	}
	firstHexRaw=TRUE;
}

void updateLastHexRaw(char * _data, typeAeiTime * t, uint32_t speed, int direction, bool first){
	int i=INDEX_AUX_DATA_HEXTAG;
	if(first){
		for(int j=0; j<SIZE_HEXTAG; j++){
			lastHexTagRx.data[j]=_data[j];
		}
		lastHexTagRx.reads=getDecFrmCharHex(_data[i+3])+getDecFrmCharHex(_data[i+2])*16;
		lastHexTagRx.rssi=getDecFrmChar(_data[i+8])+getDecFrmChar(_data[i+7])*10+getDecFrmChar(_data[i+6])*100;
		lastHexTagRx.antenna=getDecFrmChar(_data[i]);
		lastHexTagRx.speed=speed;
		lastHexTagRx.direction=direction;
		copyAeiTime(& lastHexTagRx.t, t);
		copyAeiTime(& lastHexTagRx.tEnd, t);
		qtyRepTags=1;
	}
	else{
		lastHexTagRx.reads+=getDecFrmCharHex(_data[i+3])+getDecFrmCharHex(_data[i+2])*16;
		lastHexTagRx.rssi+=getDecFrmChar(_data[i+8])+getDecFrmChar(_data[i+7])*10+getDecFrmChar(_data[i+6])*100;
		lastHexTagRx.speed+=speed;
		copyAeiTime(& lastHexTagRx.tEnd, t);
		qtyRepTags++;
	}
}

bool cmpHexRaw(char * rawHex1, char * rawHex2){
	for(int i=0; i<SIZE_HEX_HEXTAG; i++){
		if(rawHex1[i]!=rawHex2[i]){
			return FALSE;
		}
	}
	if(rawHex1[INDEX_AUX_DATA_HEXTAG]!=rawHex2[INDEX_AUX_DATA_HEXTAG])
		return FALSE;
	return TRUE;
}

void saveHexTagToFifo(){
	char asciiTimeI[30], asciiTimeE[30];

	typeHexTag* tagToSave = (typeHexTag*)malloc(sizeof(typeHexTag));
	for(int i=0; i<SIZE_HEXTAG; i++)
		tagToSave->data[i]=lastHexTagRx.data[i];
	tagToSave->speed=lastHexTagRx.speed/qtyRepTags;
	tagToSave->direction=lastHexTagRx.direction;
	tagToSave->antenna=lastHexTagRx.antenna;
	tagToSave->reads=lastHexTagRx.reads;
	tagToSave->rssi=(lastHexTagRx.rssi/qtyRepTags)*(-1);
	copyAeiTime(& tagToSave->t, & lastHexTagRx.t);
	copyAeiTime(& tagToSave->tEnd, & lastHexTagRx.tEnd);

	hexTags.Add((void *)tagToSave);

	convertTimeToASCII(& tagToSave->t, asciiTimeI);
	convertTimeToASCII(& tagToSave->tEnd, asciiTimeE);
	logg.newPrint("\nSave to Fifo: %s  %s  %s, antenna=%d, reads=%d,  rssi=%d,  speed=%lu,  dir=%d", asciiTimeI, asciiTimeE, tagToSave->data, tagToSave->antenna, tagToSave->reads, tagToSave->rssi, tagToSave->speed, tagToSave->direction);
	tagToSave=NULL;
}

void saveRawHexTagToFifo(char * rawHex, typeAeiTime * t){
	typeHexTag* tagToSave = (typeHexTag*)malloc(sizeof(typeHexTag));
	for(int i=0; i<SIZE_HEXTAG; i++)
		tagToSave->data[i]=rawHex[i];
	copyAeiTime(& tagToSave->t, t);
	hexTags.Add((void *)tagToSave);
}

void saveHexTag(char * tag, typeAeiTime * t, uint32_t speed, int direction){
	/*if(foundEndTag){//Here because if there are a tag after the ENDFOUND TAG we will deprecated this one
		return;
	}*/
	if(cmpHexRaw(startTag.data, tag)){							//Check if it is START Tag
		initLastHexRaw();
		saveRawHexTagToFifo(tag, t);
		/*if(!foundStartTag){
			saveRawHexTagToFifo(tag, t);							//Save START Tag
		}*/
	}
	else if(cmpHexRaw(endTag.data, tag)){						//Check if it is END Tag
		if(!firstHexRaw)
			saveHexTagToFifo();									//Saves lastTagRx and End tag only if flag firstHexRaw is not TRUE
		saveRawHexTagToFifo(tag, t);							//Save END Tag
	}
	else{
		if(!cmpHexRaw(lastHexTagRx.data, tag)){				//Check if tag is the same as last received
			//It is a new tag
			if(firstHexRaw){					//Check if it is the very first tag
				firstHexRaw=FALSE;
			}
			else{								//it is different but not first, save last one to Fifo and update lastRxTag
				saveHexTagToFifo();
			}
			updateLastHexRaw(tag, t, speed, direction, TRUE);
		}
		else{												//is the same tag, update data
			updateLastHexRaw(tag, t, speed, direction, FALSE);
		}
	}
}

/*
void saveHexTag(char * tag, typeAeiTime * t, uint32_t speed, int direction){
	typeHexTag* tagToSave = (typeHexTag*)malloc(sizeof(typeHexTag));
	for(int i=0; i<SIZE_HEXTAG; i++)
		tagToSave->data[i]=tag[i];
	copyAeiTime(& tagToSave->t, t);
	tagToSave->speed=speed;
	tagToSave->direction=direction;
	hexTags.Add((void *)tagToSave);
}
*/

void decEncHexToAar(){
	typeHexTag * newTag;
	char newHexTag[SIZE_HEX_HEXTAG+1];
	char newAuxData[SIZE_AUX_DATA_HEXTAG+1];
	typeAarDec * newTagAar;
	char asciiFormatS[30];
	char asciiFormatE[30];

	newTag = (typeHexTag *)hexTags.Get();
	if(newTag != NULL){
		bool tagZeroed=FALSE;
		bool saveAarTag=FALSE;
		newTagAar = (typeAarDec *)malloc(sizeof (typeAarDec));
		//Verify if tag indicates start or end of train
		if(!cmpHexTags(& startTag, newTag) && !cmpHexTags(& endTag, newTag)){
			tagZeroed = getHexMsg(newTag->data, newHexTag, newAuxData);
			convertTimeToASCII(& newTag->t, asciiFormatS);
			convertTimeToASCII(& newTag->tEnd, asciiFormatE);
			logg.newPrint("\nProc TAG.TimeRx: %s to %s   HEX: %s, AUX DATA: %d.%d.%d,    SPEED: %lu,  DIRECTION %d", asciiFormatS, asciiFormatE, newHexTag, newTag->antenna, newTag->reads, newTag->rssi, newTag->speed, newTag->direction);

			if(tagZeroed){
				logg.newPrint(";    Tag zeroed, discarding");
			}
			else{
				char binaries[1024];
				hexToBit(newHexTag, binaries);//Covert hex to binary

				if(processBin(binaries, newTagAar)){
					processAuxData(newAuxData, newTagAar);
					newTagAar->antennaNumber=newTag->antenna;
					newTagAar->timesRead=newTag->reads;
					newTagAar->rssi=newTag->rssi;
					newTagAar->speed=newTag->speed;
					newTagAar->direction=newTag->direction;
					copyAeiTime(& newTagAar->timeFirstRead, & newTag->t);
					copyAeiTime(& newTagAar->timeLastRead, & newTag->tEnd);
					saveAarTag=TRUE;
				}
				else{
					logg.newPrint(";    discarded, dataFmtCode:%d, groupCode:%d, tagType:%d", newTagAar->dataFormatCode, newTagAar->groupCode, newTagAar->tagType);
				}
			}
			//Information from hexTag is in the AAR tag
		}
		else{
			logg.newPrint("\nProc TAG.      START-END TAG FOUND");
		}

		if(saveAarTag){
			logg.newPrint("\nSaving TAG: dataFmtCode:%d, groupCode:%d, tagType:%d, frameNumber:%d", newTagAar->dataFormatCode, newTagAar->groupCode, newTagAar->tagType, newTagAar->frameNumber);
			aarTags.Add((void *)newTagAar); // saves in fifo
		}
		else
			free(newTagAar);

		hexTags.MoveReader(TRUE);
	}
}

void processorFunction(void * taskData){
	while(1){
		switch(stateTagProc){
		case stProcIni:
			OSTimeDly(TICKS_PER_SECOND*10);  //10 SEC
			stateTagProc=stProcWait;
			break;
		case stProcWait:
			OSTimeDly(TICKS_PER_SECOND * 3);  //1 SEC
			if(!sargasR.getReading() && hexTags.FifoSize()>0)
				stateTagProc=stProcDelayToCodeAar;
			break;
		case stProcDelayToCodeAar:
			OSTimeDly(4);  				//200 milisegundos
			stateTagProc=stProcCodeAar;
			break;
		case stProcCodeAar:
			OSTimeDly(1); // 50 mili
			logg.newPrint("\n\nStarting process %d Hex Tags saved",  hexTags.FifoSize());
			initIncAarTag();
			while(hexTags.FifoSize()>0) {
				decEncHexToAar();
			}
			logg.newPrint("\nFinished process Hex Tags saved");
			stateTagProc=stProcTrain;
			break;
		case stProcTrain:
			OSTimeDly(1);	// 50 mili
			createAndSaveTrain(&aarTags);
			stateTagProc=stProcAxles;
			break;

		case stProcAxles:
			OSTimeDly(1);	// 50 mili

			stateTagProc=stReportToServer;
			break;

		case stReportToServer:
			OSTimeDly(1);	// 50 mili
			hexTags.Initiate();
			aarTags.Initiate();
			//rreCars.Initiate();
			//startRep2Server();
			stateTagProc=stProcWait;
			break;
		}
	}
}

void initProcessor(){
	initStartEndTag(& startTag, & endTag);
	stateTagProc=stProcIni;
}

void addStartEndTag(bool start){
	if(start){
		if(foundStartTag){//JUST TO SAVE ONE START TAG (WHEN THERE ARE TWO SARGAS, IT SEND TWO STARTEND FOUND TAG
			logg.newPrint("\n *******ERROR FOUND DOUBLE START-TAG*********");
		}
		saveHexTag(startTag.data, & startTag.t, 0, 0);
		foundStartTag = true;
		foundEndTag = false;
	}else{
		if(foundEndTag){//JUST TO SAVE ONE START TAG (WHEN THERE ARE TWO SARGAS, IT SEND TWO STARTEND FOUND TAG
			logg.newPrint("\n *******ERROR FOUND DOUBLE END-TAG*********");
		}
		saveHexTag(endTag.data, & endTag.t, 0, 0);
		foundEndTag = true;
		foundStartTag = false;
	}
}

void startProcessor(){
	OSSimpleTaskCreatewName(processorFunction,TAG_PROCESS_PRIO,"TAGPROCESSOR");
}


