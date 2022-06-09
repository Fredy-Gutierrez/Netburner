/*
 * tagUtilities.h
 *
 *  Created on: 12 abr 2021
 *      Author: Integra Fredy
 */

#ifndef TAGUTILITIES_H_
#define TAGUTILITIES_H_

#include "libs/Rtc_Controller.h"

#define SIZE_HEXTAG				64
#define SIZE_HEX_HEXTAG			30
#define SIZE_AUX_DATA_HEXTAG	9
#define INDEX_AUX_DATA_HEXTAG	54

#define fmtCodeDyn				37
#define fmtCodeNonDyn			51
#define typTagNonDyn			2
#define typTagMultiFrag			3
#define typTagReserved			4
#define eqGrpCodeLocomotive		5
#define eqGrpCodeEot			6
#define eqGrpCodeRailcar		19


//#define MaxBuffer (1000)
#define BASE (27)
#define BASEe2 (729)
#define BASEe3 (19683)
#define START_END_INDICATOR		0x555

enum typeOfData {dtAarHexData,  dtAarDecData, dtT94Rre, dtT94EOT, dtAxle};

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */



	const char C1[] = {	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
						'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z','\0'};
	const char C2_4[] = {	' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
							'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z','\0'};

	typedef struct{
		char data[SIZE_HEXTAG];
		typeAeiTime t;
		typeAeiTime tEnd;
		uint32_t speed;	//speed in deca meters by hour
		int direction;	//1=FORWARD, -1=BACKWARD
		int antenna;
		int reads;
		int rssi;
	}typeHexTag;

	typedef struct{
		int groupCode;
		int tagType;
		int eotNumber;
		int eotType;
		int sideIndicator;
		char equipmentInitial[5];
	}typeEndOfTrain;

	typedef struct{
		int par;
		int rep;
	}typeDylPar;
#define qtyDylPar		7

	typedef struct{
		int groupCode;
		int tagType;
		int carNumber;
		int sideIndicator;
		int decimetersLength;
		int axlesNumber;
		int bearingCode;
		int platformCode;
		//int spare;
		int reserved;
		//int security;
		int dataFormatCode;
		int antennaNumber;
		int	timesRead;
		int rssi;
		char equipmentInitial[5];
		typeAeiTime timeFirstRead;
		typeAeiTime timeLastRead;
		//typeAeiTime utrRxTime;
		uint32_t speed;
		int direction;
		int eotType;
		int alarmCodes;
		int volFuelTank;
		int cumKwHr;
		typeDylPar dylPar[qtyDylPar];
		int commsStatus;
		int etsSwitch;
		int frameNumber;
	}typeAarDec;

	typedef struct{
		char segmentID[3];
		char sequenceNumber[3];
		char groupCode[1];
		char ownerCode[4];
		char ownerEquipmentNumber[10];
		char orientation[1];
		char reserved[1];
		char axlesConversionCode[1];
		char tagStatus[1];
		char tagDetailStatus[1];
		char antena0Count[2];
		char antena1Count[2];
		char speedVehicle[3];
		char axlesCount[3];
		char platformCount[3];
		char separator1[1];
		char direction[1];
		char length[4];
		char tagBearingCode[1];
		char tagPlatformCode[2];
		char separator2[1];						//Last parameter to be send to server
		typeAeiTime startTime;					//Times must be converteed to Asccii
		typeAeiTime endTime;
		int axlesCountTag;						//From here initiate parameters saved in a different format for convenience
		int carNumberTag;
		int speedTag;
		int ant0Count;
		int ant1Count;
	}typeT94Rre;

	typedef struct{
		char segmentRre[49]; //Not includes '\0'
		char timeIni[23];
		char separator[1];
		char timeEnd[23];
		char tail[1];
	}typeT94RreTimed;

	typedef char typeT94RrePost[97];

/*	typedef struct{
		int direction;
	}headerT94;
*/
	typedef struct{
		char segmentID[3];					//Fixed
		char billingCode[5];				//Fixed
		char siteId[7];						//Extern
		char startDate[6];
		char startTime[4];
		char stopTime[4];
		char timeZone[3];					//Fixed
		char daylightSavingstime[1];		//Fixed
		char dataFormatVersion[3];			//Fixed
		char trainSeqNumber[4];				//Extern
		char locomotiveConversionStatus[1];	//Fixed
		char railcarConversionStatus[1];	//Fixed
		char directionOfTravel[1];			//Fixed
		char switchIndicator[1];			//Fixed
		char unitsOfMeasure[1];				//Fixed
		char maxSpeed[3];
		char minSpeed[3];
		char avSpeed[3];
		char movementStatus[1];
		char terminationStatus[1];			//Fixed
		char transmissionType[1];			//Fixed
		char adjacentTrackOccupied[1];		//Fixed
		char trainLength[5];
		char equipmentStatusCode[1];		//Fixed
		char locomotiveCount[2];
		char locomotivesTagged[2];
		char railcarCount[3];
		char railcarsTagged[3];
		char totalAxleCount[4];
		char separator[1];					//Last parameter to be send to server
		typeAeiTime iStartTime;				//Times must be converteed to Asccii
		typeAeiTime iStopTime;
		int iMaxSpeed;						//From here initiate parameters saved in a different format for convenience
		int iMinSpeed;
		int iAvSpeed;
		int iTrainLength;
		int iLocomotiveCount;
		int iLocomotivesTagged;
		int iRailcarCount;
		int iRailcarsTagged;
		int iTotalAxleCount;
		bool iOneDirection;
	}typeT94Aem;

	typedef char typeT94AemPost[78];

	typedef struct{
		char segmentID[3];
		char sequenceNumber[3];
		char groupCode[1];
		char ownerCode[4];
		char ownerEquipmentNumber[10];
		char antena0Count[2];
		char antena1Count[2];
		char tagDetailStatus[1];
		char separator[1];						//Last parameter to be send to server
		typeAeiTime startTime;					//Times must be converteed to Asccii
		typeAeiTime endTime;
		int ant0Count;							//From here initiate parameters saved in a different format for convenience
		int ant1Count;
	}typeT94Eot;

	typedef struct{
		char segmentEot[27];
		char timeIni[23];
		char separator[1];
		char timeEnd[23];
		char tail[1];
	}typeT94EotTimed;

	typedef char typeT94EotPost[75];

	typedef struct{
		char segmentID[3];
		char totalBytesCount[10];
		char separator[1];						//Last parameter to be send to server
		typeAeiTime startTime;					//Times must be converteed to Asccii
		typeAeiTime endTime;
		int iTotalBytesCount;
	}typeT94Eoc;

	typedef struct{
		char segmentEoc[14];
		char timeIni[23];
		char separator[1];
		char timeEnd[23];
		char tail[1];
	}typeT94EocTimed;

	typedef char typeT94EocPost[62];

	typedef struct{
		char segmentID[3];
		char sequenceNumber[3];
		char groupCode[1];
		char ownerCode[4];
		char ownerEquipmentNumber[10];
		char tagDetailStatus[1];
		char antena0Count[2];
		char antena1Count[2];
		char alarmCodes[2];
		char volumeFuelTank[3];
		char cumulativeKwHours[5];
		char etsSwitch[2];
		char dylPar[7][1];
		char commsIndicator[1];
		char thirdTagIndicator[1];
		char separator1[1];
		char direction[1];
		char length[4];
		char tagBearingCode[1];
		char separator2[1];				//Last parameter to be send to server
		typeAeiTime startTime;			//Times must be converteed to Asccii
		typeAeiTime endTime;
		int axlesCountTag;				//From here initiate parameters saved in a different format for convenience
		int carNumberTag;
		int speedTag;
		int ant0Count;
		int ant1Count;
	}typeT94Dyl;

	typedef struct{
		char segmentDyl[62]; //Not includes '\0'
		char timeIni[23];
		char separator[1];
		char timeEnd[23];
		char tail[1];
	}typeT94DylTimed;

	typedef char typeT94DylPost[110];

	#define MAX_RRE_TRAIN			250
	enum stReportTrain {stNotReady, stReady, stSegAem, stSegRre, stSegEot, stSegEoc, stReported};
	typedef struct{
		bool reportedFlag;
		typeT94Aem segAem;
		typeT94Eoc segEoc;
		typeT94Eot segEot;
		typeT94Dyl segDyl;
		bool hasSegEot;
		int idxRre;
		int stateOfReport;
		int qtyBytes;
		int qtyRre;
		typeT94Rre segRre[MAX_RRE_TRAIN];
	}typeTrain;

	enum typeTagStatus {bothTags='G', mismatchId='M', leftTagMissing='L', rightTagMissing='R', noTagRead='N' };

	void hexToBit(char input[], char * res);
	long long int binaryToDecimal(char binaryInput[]);
	char * getInitials(int val);
	bool processBin(char bin[], typeAarDec *tag);
	void processAuxData(char auxData[], typeAarDec *tag);
	bool getHexMsg(const char * tag, char *hexTag, char *tagEnd);
	bool cmpTag(typeAarDec * tag1, typeAarDec * tag2);
	void incSaveAuxData(typeAarDec * lastTag, typeAarDec * newTag);
	bool updateIncAarTag(typeAarDec * rxTag, typeAeiTime * timeRx);
	void initIncAarTag();
	void initStartEndTag(typeHexTag * sTag, typeHexTag * eTag);
	bool cmpHexTags(typeHexTag * tag1, typeHexTag * tag2);
	typeT94Rre * makeCars(typeAarDec * tag, int * seqNumber, bool forceCarSave);
	bool getCarsT94RreFmted(typeT94RrePost * pCarToGet);
	int getDecFrmCharHex(char c);
	int getDecFrmChar(char c);
	void convIntToAscii(int value, char ascii[], int qtyDigits);

	void printAarTag(typeAarDec *tagPrn);
	void printRreCar(typeT94Rre *carPrn);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* TAGUTILITIES_H_ */
