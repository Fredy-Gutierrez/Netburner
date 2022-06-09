/*
 * SDCardDriver.h
 *
 *  Created on: 24 mar 2022
 *      Author: Integra Fredy
 */

#ifndef LIBS_SDCARDDRIVER_H_
#define LIBS_SDCARDDRIVER_H_


#include <effs_fat/fat.h>

#define USE_MMC         // SD/MMC cards

#if (defined USE_MMC)
	#include <effs_fat/mmc_mcf.h>
	#define EXT_FLASH_DRV_NUM (MMC_DRV_NUM)
#endif

#define MAX_EFFS_ERRORCODE (38)
extern char EffsErrorCode[][80];

#define MAX_FILE_NAME 75

// FAT Media Types for Format
#define F_FAT12_FORMAT (1)
#define F_FAT16_FORMAT (2)
#define F_FAT32_FORMAT (3)

#ifdef __cplusplus
extern "C"
{
#endif

    void DisplayEffsErrorCode(int code);

    int AddFSTask(uint8_t taskPrio = 0, uint8_t actualPrio = 0);
	void CloseFSTask(uint8_t taskPrio = 0, uint8_t actualPrio = 0);

	int InitExtFlash();
	uint8_t UnmountExtFlash();
	uint8_t FormatExtFlash(long FATtype = F_FAT32_FORMAT);

	bool MakeDir(const char  * dirName);
	bool MoveToDirectory(const char  * dirName);

	FN_FILE * OpenFile(char * pfileName, char * openMode);
	FN_FILE * CreateAndOpenNewFile(char * pfileName,char * pfileExt, char * openMode, char * newFileName);
	int TruncateFile(F_FILE *fp, uint32_t newSize);
	void CloseFile(F_FILE *fp);
	bool DeleteFile(char *pFileName);

	uint32_t SDWrite(void *pDataToWrite, long NumBytes, F_FILE *fp);
	uint32_t SDRead(void *pReadBuffer, uint32_t NumBytes, F_FILE *fp);

	void SetPointerToStart(F_FILE *fp);
	void SetPointerToEnd(F_FILE *fp);
	void SetPointerTo(F_FILE *fp, uint32_t position);

#ifdef __cplusplus
}
#endif


#endif /* LIBS_SDCARDDRIVER_H_ */
