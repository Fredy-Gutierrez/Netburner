/*
 * SDCardDriver.cpp
 *
 *  Created on: 24 mar 2022
 *      Author: Integra Fredy
 */
#include <predef.h>
#include <basictypes.h>
#include <ctype.h>
#include <nbrtos.h>
#include <stdio.h>
#include <utils.h>
#include <effs_fat/effs_utils.h>
#include <effs_fat/fat.h>
#include "libs/SDCardDriver.h"
#include <string.h>
#include "libs/Rtc_Controller.h"
#include "libs/Log.h"
#include "libs/Business.h"

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

/*-------------------------------------------------------------------
  DisplayEffsErrorCode()
 -------------------------------------------------------------------*/
void DisplayEffsErrorCode(int code)
{
    if (code <= MAX_EFFS_ERRORCODE) { logg.newPrint("%s\r\n", EffsErrorCode[code]); }
    else
    {
    	logg.newPrint("Unknown EFFS error code [%d]\r\n", code);
    }
}

/*-------------------------------------------------------------------
  AddFSTask() - Add a new task priority in task list to use the
  External SD Card, if the task that want to use the SD Card utiliies
  have not added her task it will not can use the SD Card

  * @taskPrio			-> 	Task priority that you want to add in list
   								- Default 0 to use the callback task priority

  * @actualPrio			-> 	Task priority of the callback
  	  	  	  	  	  	  	  	- Default 0 to use the callback task priority
 -------------------------------------------------------------------*/
int AddFSTask(uint8_t taskPrio, uint8_t actualPrio){
	if(taskPrio != 0){
		OSChangePrio(taskPrio);    // change to this task's priority
	}
	int result = f_enterFS();
	if(actualPrio != 0){
		OSChangePrio(actualPrio);   // change back to this task's priority
	}
	return result;
}

/*-------------------------------------------------------------------
  CloseFSTask() - Removes task priority from task list which have all
  task that can use the SD Card

  * @taskPrio			-> 	Task priority that you want to add in list
   								- Default 0 to use the callback task priority

  * @actualPrio			-> 	Task priority of the callback
  	  	  	  	  	  	  	  	- Default 0 to use the callback task priority
 -------------------------------------------------------------------*/
void CloseFSTask(uint8_t taskPrio, uint8_t actualPrio){
	if(taskPrio != 0){
		OSChangePrio(taskPrio);
	}
	f_releaseFS();
	if(actualPrio != 0){
		OSChangePrio(actualPrio);   // change back to this task's priority
	}
}

/*-------------------------------------------------------------------
  Init() - Initialization function for setting up EFFS on
  MultiMedia/Secure Digital cards or Compact Flash cards.
  @returned value	->	0 for success !0 for error
 -------------------------------------------------------------------*/
int InitExtFlash()
{
	#if (defined USE_MMC)
		if(get_cd()==0){
			logg.newPrint("\n*** Error: ");
			DisplayEffsErrorCode(F_ERR_CARDREMOVED);
			return F_ERR_CARDREMOVED;
		}else{
			if(get_wp()==1){
				logg.newPrint("\n*** Error: ");
				DisplayEffsErrorCode(F_ERR_WRITEPROTECT);
				return F_ERR_WRITEPROTECT;
			}
		}
	#endif

    int rv = 0;

    sniprintf(driveType, 20, "No Drive");

	#if (defined USE_MMC)
		//iprintf("Mounting drive USE_MMC mode\r\n");
		sniprintf(driveType, 20, "SD/MMC");
		rv = f_mountfat(MMC_DRV_NUM, mmc_initfunc, F_MMC_DRIVE0);
	#endif

    if(rv==F_NO_ERROR) {
		rv = f_chdrive(EXT_FLASH_DRV_NUM);
		if(rv == F_NO_ERROR) {
			logg.newPrint("\nSD MOUNTED\r\n");
			return F_NO_ERROR;
		}
		else{
			//iprintf("%s drive change failed: ", driveType);
			logg.newPrint("\n*** Error: ");
			DisplayEffsErrorCode(rv);
			return rv;
		}
    }
    else{
    	logg.newPrint("\n*** Error: ");
    	DisplayEffsErrorCode(rv);
    	return rv;
    }
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
    	logg.newPrint("\n*** Error in f_delvolume(): ");
        DisplayEffsErrorCode(rv);
        return 0;
    }
    logg.newPrint("\nSD Unmounted\r\n\r\n");
    return 1;
}
uint8_t FormatExtFlash(long FATtype)
{
    int rv;
    logg.newPrint("\nFormatting %s card\r\n\r\n", driveType);
    rv = f_format(EXT_FLASH_DRV_NUM, FATtype);
    if (rv != F_NO_ERROR)
    {
    	logg.newPrint("\n*** Error in f_format(): ");
        DisplayEffsErrorCode(rv);
    }
    return rv;
}

bool MakeDir(const char * dirName){
	int rv = f_mkdir(dirName);
	if(rv != 0 && rv != 6){
		DisplayEffsErrorCode(rv);
		return false;
	}
	return true;
}
bool MoveToDirectory(const char * dirName){
	int rv = f_chdir(dirName);
	if(rv != 0){
		DisplayEffsErrorCode(rv);
		return false;
	}
	return true;
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
FN_FILE * OpenFile(char * pfileName, char * openMode){
	FN_FILE * fpUser = NULL;
	if(pfileName != NULL){
		fpUser = f_open(pfileName, openMode);
	}
	return fpUser;
}

/*TruncateFile a open file with a determinate bytes
 *  @fp				-> 	File pointer to close
 *  @newSize		-> 	New file size
*/
int TruncateFile(F_FILE *fp, uint32_t newSize){
	if(fp != NULL){
		return f_ftruncate(fp, newSize);
	}
	return -1;
}
/*Close the open file
 *  @fp				-> 	File pointer to clise
*/
void CloseFile(F_FILE *fp){
	if(fp != NULL){
		volatile int rv;
		rv = f_close(fp);
		if(rv != F_NO_ERROR){
			logg.newPrint("\nThe file could not be closed\n");
		}
	}
}
/*Delete and existing file
 *  @fp				-> 	File pointer to rewind
* @returned value 	->	true for success, false for faile
*/
bool DeleteFile(char *pFileName)
{
    volatile int rv;
    rv = f_delete(pFileName);
    if (rv != F_NO_ERROR)
    {
    	logg.newPrint("\nThe file %s could not be deleted\n",pFileName);
    }
    return rv == F_NO_ERROR;
}

/*write n number of bytes in the current pointer position, if the sdcard is not mounted it mount this
 * if the file is not open it open this.
 * @pDataToWrite 	-> 	Data to be written in file
 * @NumBytes		-> 	Number of bytes to be written
 * @fp				-> 	File pointer
 *
 * @returned value 	->	Number of bytes written*/
uint32_t SDWrite(void *pDataToWrite, long NumBytes, F_FILE *fp){
	uint32_t wBytes = 0;
	if(fp != NULL){
		wBytes = f_write(pDataToWrite, NumBytes, 1, fp);
	}
	return wBytes;
}
/*read n number of bytes in the current pointer position
 * @pReadBuffer		-> 	buffer to storage the data from file
 * @NumBytes		-> 	Number of bytes to read
 * @fp				-> 	File pointer
 *
 * @returned value 	->	Number of bytes read*/
uint32_t SDRead(void *pReadBuffer, uint32_t NumBytes, F_FILE *fp){
	uint32_t rBytes = 1;
	if(fp != NULL && !f_eof(fp)){
		f_read(pReadBuffer, NumBytes, 1, fp);
	}else{
		rBytes = 0;
	}
	return rBytes;
}

/*set current file pointer to start of file
 *
 * @fp				-> 	File pointer to rewind
*/
void SetPointerToStart(F_FILE *fp){
	volatile int rv;
	if(fp != NULL){
		rv = f_rewind(fp);
		if(rv != F_NO_ERROR){
			logg.newPrint("\nThe pointer could not be moved at start\n");
		}
	}
}
/*set current file pointer to start of file
 *
 * @fp				-> 	File pointer to seek
*/
void SetPointerToEnd(F_FILE *fp){
	volatile int rv;
	if(fp != NULL){
		rv = f_seek(fp, 0, FN_SEEK_END);
		if(rv != F_NO_ERROR){
			logg.newPrint("\nThe pointer could not be moved at end\n");
		}
	}
}
/*set current file pointer at new position determinated by the bytes quantity to move
 *
 * @fp				-> 	File pointer to rewind
 * @NumBytes		-> 	Number of bytes to move
 * */
void SetPointerTo(F_FILE *fp, uint32_t position){
	volatile int rv;
	if(fp != NULL){
		rv = f_seek(fp, position, FN_SEEK_SET);
		if(rv != F_NO_ERROR){
			logg.newPrint("\nThe pointer could not be moved at position\n");
		}
	}
}


