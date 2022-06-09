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

/*------------------------------------------------------------------------------
 * EFFS FAT32 file system utilities for use with SD and microSD flash cards.
 *
 * Modules with an onboard microSD flash socket should use the multi mmc header
 * files and functions because the modules are capable of supporting both onboard
 * and external flash cards (even if you application only uses one).
 *
 ------------------------------------------------------------------------------*/

#include <predef.h>
#include <basictypes.h>
#include <ctype.h>
#include <nbrtos.h>
#include <stdio.h>
#include <utils.h>
#include <effs_fat/effs_utils.h>
#include <effs_fat/fat.h>
#include "FatLib/cardtype.h"
#include "FatLib/FileSystemUtils.h"
#include <string.h>

char driveType[20];

char EffsErrorCode[][80] = {
    "F_NO_ERROR",                // 0
    "F_ERR_INVALIDDRIVE",        // 1
    "F_ERR_NOTFORMATTED",        // 2
    "F_ERR_INVALIDDIR",          // 3
    "F_ERR_INVALIDNAME",         // 4
    "F_ERR_NOTFOUND",            // 5
    "F_ERR_DUPLICATED",          // 6
    "F_ERR_NOMOREENTRY",         // 7
    "F_ERR_NOTOPEN",             // 8
    "F_ERR_EOF",                 // 9
    "F_ERR_RESERVED",            // 10
    "F_ERR_NOTUSEABLE",          // 11
    "F_ERR_LOCKED",              // 12
    "F_ERR_ACCESSDENIED",        // 13
    "F_ERR_NOTEMPTY",            // 14
    "F_ERR_INITFUNC",            // 15
    "F_ERR_CARDREMOVED",         // 16
    "F_ERR_ONDRIVE",             // 17
    "F_ERR_INVALIDSECTOR",       // 18
    "F_ERR_READ",                // 19
    "F_ERR_WRITE",               // 20
    "F_ERR_INVALIDMEDIA",        // 21
    "F_ERR_BUSY",                // 22
    "F_ERR_WRITEPROTECT",        // 23
    "F_ERR_INVFATTYPE",          // 24
    "F_ERR_MEDIATOOSMALL",       // 25
    "F_ERR_MEDIATOOLARGE",       // 26
    "F_ERR_NOTSUPPSECTORSIZE",   // 27
    "F_ERR_DELFUNC",             // 28
    "F_ERR_MOUNTED",             // 29
    "F_ERR_TOOLONGNAME",         // 30
    "F_ERR_NOTFORREAD",          // 31
    "F_ERR_DELFUNC",             // 32
    "F_ERR_ALLOCATION",          // 33
    "F_ERR_INVALIDPOS",          // 34
    "F_ERR_NOMORETASK",          // 35
    "F_ERR_NOTAVAILABLE",        // 36
    "F_ERR_TASKNOTFOUND",        // 37
    "F_ERR_UNUSABLE"             // 38
};

SD_STATUS sd_status = SD_NOT_STARTED;

/*-------------------------------------------------------------------
  DisplayEffsErrorCode()
 -------------------------------------------------------------------*/
void DisplayEffsErrorCode(int code)
{
    if (code <= MAX_EFFS_ERRORCODE) { iprintf("%s\r\n", EffsErrorCode[code]); }
    else
    {
        iprintf("Unknown EFFS error code [%d]\r\n", code);
    }
}

/*-------------------------------------------------------------------
  Init() - Initialization function for setting up EFFS on
  MultiMedia/Secure Digital cards or Compact Flash cards.
  @returned value	->	0 for success !0 for error
 -------------------------------------------------------------------*/
SD_STATUS InitExtFlash()
{
#if (defined USE_MMC)
	if(get_cd()==0){
		iprintf("*** Error: ");
		DisplayEffsErrorCode(F_ERR_CARDREMOVED);
		return SD_NOT_FOUND;
	}else{
		sd_status = SD_FOUND;
		if(get_wp()==1){
			iprintf("*** Error: ");
			DisplayEffsErrorCode(F_ERR_WRITEPROTECT);
			return SD_NOT_STARTED;
		}
	}
#endif

    int rv = 0;
    /* The f_mountfat() function is called to mount a FAT drive. Although
       there are parameters in this function, they should not be modified.
       The function call should be used as it appears for a Compact Flash
       card. For reference, the parameters are:

       drive_num:   Set to MMC_DRV_NUM, the drive to be mounted
       p_init_func: Points to the initialization function for the drive.
                    For the Compact Flash drive, the function is located
                    in \nburn\platform\<platform>\source\mmc.c.
       p_user_info: Used to pass optional information. In this case, the
                    drive number.

       The return values are:
          F_NO_ERROR:  drive successfully mounted
          Any other value: Error occurred. See page 22 in the HCC-Embedded
          file system manual for the list of error codes.
    */
    sniprintf(driveType, 20, "No Drive");
#if (defined USE_MMC)
    //iprintf("Mounting drive USE_MMC mode\r\n");
    sniprintf(driveType, 20, "SD/MMC");
    rv = f_mountfat(MMC_DRV_NUM, mmc_initfunc, F_MMC_DRIVE0);
#endif

    if(rv==F_NO_ERROR) {
		rv = f_chdrive(EXT_FLASH_DRV_NUM);
		if(rv == F_NO_ERROR) {
			iprintf("SD MOUNTED\r\n");
			return SD_STARTED;
		}
		else{
			//iprintf("%s drive change failed: ", driveType);
			iprintf("*** Error: ");
			DisplayEffsErrorCode(rv);
			return SD_NOT_STARTED;
		}
    }
    else{
    	iprintf("*** Error: ");
    	DisplayEffsErrorCode(rv);
    	return SD_NOT_STARTED;
    }
    /* Change to SD/MMC drive
       We need to call the change function to access the new drive. Note
       that ANY function other than the f_mountfat() is specific to a task.
       This means that if f_chdrive() is called in a different task to a
       different drive, it will not affect this task - this task will
       remain on the same drive.
    */
}

/*-------------------------------------------------------------------
uint8_t UnmountSD()
 -------------------------------------------------------------------*/
uint8_t UnmountExtFlash()
{
    int rv = 0;
    rv = f_delvolume(EXT_FLASH_DRV_NUM);
    if (rv != F_NO_ERROR)
    {
        iprintf("*** Error in f_delvolume(): ");
        DisplayEffsErrorCode(rv);
        return 0;
    }
    iprintf("SD Unmounted\r\n\r\n");
    return 1;
}

int addFSTask(uint8_t taskPrio, uint8_t actualPrio){
	if(taskPrio != 0){
		OSChangePrio(taskPrio);
	}
	int result = f_enterFS();
	if(actualPrio != 0){
		OSChangePrio(actualPrio);   // change back to this task's priority
	}
	return result;
}

/*mounts and starts the sd card to read/write files*/
SD_STATUS initSDCard(){
	sd_status = InitExtFlash();   // Initialize the CFC or SD/MMC external flash drive
	return sd_status;
}

/*Close and unmount the sd card to be able to remove*/
void closeSDCard(){
	UnmountExtFlash();
	sd_status = SD_NOT_STARTED;
}

/*Close and unmount the sd card to be able to remove*/
void closeFSTask(uint8_t taskPrio, uint8_t actualPrio){
	if(taskPrio != 0){
		OSChangePrio(taskPrio);
	}
	f_releaseFS();
	if(actualPrio != 0){
		OSChangePrio(actualPrio);   // change back to this task's priority
	}
}

/*Close the open file*/
void closeFile(F_FILE *fp){
	if(fp != NULL){
		volatile int rv;
		rv = f_close(fp);
		if(rv != F_NO_ERROR){
			iprintf("The file could not be closed\n");
		}
	}
}

uint8_t FormatExtFlash(long FATtype)
{
    int rv;
    iprintf("Formatting %s card\r\n\r\n", driveType);
    rv = f_format(EXT_FLASH_DRV_NUM, FATtype);
    if (rv != F_NO_ERROR)
    {
        iprintf("*** Error in f_format(): ");
        DisplayEffsErrorCode(rv);
    }
    return rv;
}

bool DeleteFile(char *pFileName)
{
	if(sd_status != SD_STARTED){ if(initSDCard() != SD_STARTED){ return false; } }
    volatile int rv;
    rv = f_delete(pFileName);
    if (rv != F_NO_ERROR)
    {
    	iprintf("The file %s could not be deleted\n",pFileName);
    }
    return rv == F_NO_ERROR;
}

/*open a file for read or write, starts the sd card if it is unmounted
 * @pfileName	 	-> 	File name to be opened
 * @openMode		-> 	File open mode
 *		The f_open() function opens a file for reading or writing. The following
       	modes are allowed to open:
          "r"   Open existing file for reading. The stream is positioned at the
                beginning of the file.
          "r+"  Open existing file for reading and writing. The stream is
                positioned at the beginning of the file.
          "w"   Truncate file to zero length or create file for writing. The
                stream is positioned at the beginning of the file.
          "w+"  Open a file for reading and writing. The file is created if it
                does not exist, otherwise it is truncated. The stream is
                positioned at the beginning of the file.
          "a"   Open for appending (writing to end of file). The file is created
                if it does not exist. The stream is positioned at the end of the
                file.
          "a+"  Open for reading and appending (writing to end of file). The file
                is created if it does not exist. The stream is positioned at the
                end of the file.
 */
FN_FILE * openFile(char * pfileName, char * openMode){
	if(sd_status != SD_STARTED){
		if(initSDCard() != SD_STARTED){ return NULL; }
	}
	FN_FILE * fpUser = NULL;
	if(pfileName != NULL){
		FN_FIND fileFound;
		if(f_findfirst(pfileName, &fileFound) == F_NO_ERROR){
			fpUser = f_open(pfileName, openMode);
		}
	}
	return fpUser;
}

#define MAX_DIGITS 10
FN_FILE * createAndOpenNewFile(char * pfileName,char * pfileExt, char * openMode, char * newFileName){
	if(sd_status != SD_STARTED){
		if(initSDCard() != SD_STARTED){ return NULL; }
	}
	FN_FILE * fpUser = NULL;
	if(pfileName != NULL){
		char fileName[MAX_FILE_NAME+sizeof(char)];
		strcpy(fileName, pfileName);
		strcat(fileName, ".");
		strcat(fileName, pfileExt);
		FN_FIND fileFound;
		int count = 1;
		while(f_findfirst(fileName, &fileFound) == F_NO_ERROR){
			memset(fileName, 0, MAX_FILE_NAME * (sizeof fileName[0]) );
			char num_char[MAX_DIGITS+sizeof(char)];
			sprintf(num_char, "%d", count);
			strcpy(fileName, pfileName);
			strcat(fileName, "(");
			strcat(fileName, num_char);
			strcat(fileName, ")");
			strcat(fileName, ".");
			strcat(fileName, pfileExt);
			count++;
		}
		strcpy(newFileName,fileName);
		fpUser = f_open(fileName, openMode);
	}
	return fpUser;
}

/*set current file pointer to start of file*/
void setPointerToStart(F_FILE *fp){
	volatile int rv;
	if(fp != NULL){
		rv = f_rewind(fp);
		if(rv != F_NO_ERROR){
			iprintf("The file reader could not be started\n");
		}
	}
}

/*write n number of bytes in the current pointer position, if the sdcard is not mounted it mount this
 * if the file is not open it open this.
 * @pDataToWrite 	-> 	Data to be written in file
 * @NumBytes		-> 	Number of bytes to be written
 *
 * @returned value 	->	Number of bytes written*/
uint32_t writeText(uint8_t *pDataToWrite, long NumBytes, F_FILE *fp){
	if(sd_status != SD_STARTED){
		if(initSDCard() != SD_STARTED){ return 0; }
	}
	uint32_t wBytes = 0;
	if(fp != NULL){
		wBytes = f_write(pDataToWrite, NumBytes, 1, fp);
	}
	return wBytes;
}

/*read n number of bytes in the current pointer position, if the sdcard is not mounted it mount this
 * if the file is not open it open this.
 * @pReadBuffer		-> 	buffer to storage the data from file
 * @NumBytes		-> 	Number of bytes to read
 *
 * @returned value 	->	Number of bytes read*/
uint32_t readText(uint8_t *pReadBuffer, uint32_t NumBytes, F_FILE *fp){
	if(sd_status != SD_STARTED){
		if(initSDCard() != SD_STARTED){ return 0; }
	}
	uint32_t rBytes = 0;
	if(fp != NULL && !f_eof(fp)){
		rBytes = (uint32_t)f_read(pReadBuffer, NumBytes, 1, fp);
	}
	return rBytes;
}

