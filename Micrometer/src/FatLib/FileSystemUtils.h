/* Revision: 3.3.1 */

/******************************************************************************
* Copyright 1998-2021 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 16855 W Bernardo Dr
* San Diego, CA 92127
* www.netburner.com
******************************************************************************/

#ifndef _FILESYSUTIL_H
#define _FILESYSUTIL_H
#include <effs_fat/fat.h>
#include "FatLib/cardtype.h"

#if (defined(USE_MMC))
#include <effs_fat/mmc_mcf.h>
#elif (defined USE_SDHC)// && defined(MOD5441X))
#include <effs_fat/sdhc_mcf.h>
#endif


#define MAX_EFFS_ERRORCODE (38)
extern char EffsErrorCode[][80];
enum SD_STATUS{
	SD_NOT_FOUND,
	SD_FOUND,
	SD_NOT_STARTED,
	SD_STARTED,
	FILE_NOT_OPEN,
	FILE_OPEN
};

#define MAX_FILE_NAME 75

#ifdef __cplusplus
extern "C"
{
#endif

// FAT Media Types for Format
#define F_FAT12_FORMAT (1)
#define F_FAT16_FORMAT (2)
#define F_FAT32_FORMAT (3)

    void DisplayEffsErrorCode(int code);

    int addFSTask(uint8_t taskPrio, uint8_t actualPrio);
	void closeFile(F_FILE *fp);
	uint8_t FormatExtFlash(long FATtype = F_FAT32_FORMAT);
	bool DeleteFile(char *pFileName);
	FN_FILE * openFile(char * pfileName, char * openMode);
	void setPointerToStart();
	uint32_t writeText(uint8_t *pDataToWrite, long NumBytes, F_FILE *fp);
	uint32_t readText(uint8_t *pReadBuffer, uint32_t NumBytes, F_FILE *fp);
	FN_FILE * createAndOpenNewFile(char * pfileName,char * pfileExt, char * openMode, char * newFileName);
	void closeSDCard();
	void closeFSTask(uint8_t taskPrio, uint8_t actualPrio);
	SD_STATUS initSDCard();

#ifdef __cplusplus
}
#endif

#endif
