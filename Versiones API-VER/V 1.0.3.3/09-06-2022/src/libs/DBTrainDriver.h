/*
 * DBTrainDriver.h
 *
 *  Created on: 24 mar 2022
 *      Author: Integra Fredy
 */

#ifndef LIBS_DBTRAINDRIVER_H_
#define LIBS_DBTRAINDRIVER_H_

#include <basictypes.h>
#include <libs/TagUtilities.h>

#define BASETRAINDIR "/DB/Trains"
#define BASETRAINDATADIR "/DB/DataTrains"
#define CONTROLTRAINFILE "ControlTrain.bin"


struct TrainControl{
	char DayDirRead[9];
	uint32_t TrainNumRead;
	char DayDirWrite[9];
	uint32_t TrainNumWrite;
	uint32_t TrainToSend;
}__attribute__((packed));

enum RequestState{StNoRequest, StInProgressReq, StCompletedReq};
enum RequestType{ReadRequest, WriteRequest, OverWriteRequest};//REQUEST TYPES
enum RequestDataType{TrainData, TrainLogData};//REQUEST DATA THAT MUST BE GETTED OR SETTED
/*****************************ALL USERS THAT WANT USE THE SD CARD MUST SEND THIS STRUCTURE WITH ITS INFORMATION********************/
struct SDTrainUsers{
	RequestType request;
	RequestDataType dataType;
	void * dataBuffer;
	int buffSize;
	RequestState stateRequest;
	bool modReqStFlag;
	bool useTrainControl;
	char path[20];
	char fileName[16];
};

enum DBState{StDbInit, StDbStart, StDbWait, StDbTrain, StDbTrainLog};//controls the DBProcess

#ifdef __cplusplus
extern "C"
{
#endif
	TrainControl * getControlTrain(RequestDataType controlType);
	void printTrainControlInfo();
	uint32_t getQtyTrainsToSend();
	void makeTrainFileName(char * nFileName, int secuence);
	bool canDBRequest();
	bool newRequest(SDTrainUsers * user);
	void InitDBTrain();

#ifdef __cplusplus
}
#endif
#endif /* LIBS_DBTRAINDRIVER_H_ */
