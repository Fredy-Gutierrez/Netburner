/*
 * SpeedProcess.h
 *
 *  Created on: 20 april 2021
 *      Author: Integra MGZ
*/
#ifndef AXLEDETPROCESS_H_
#define AXLEDETPROCESS_H_

#include <basictypes.h>
#include "libs/Rtc_Controller.h"

enum speedState { stIni, stWaitSys1, stWaitSys2 };

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	//void printInputs();

#ifdef __cplusplus
}
#endif /* __cplusplus */


/***************************************************** Class speedProc **********************************************
*/
class axleDetProc {
public:
	uint32_t format;
	int dir;
	uint32_t state;
	uint32_t digSys1;
	uint32_t digSys2;
	typeAeiTime onSys1;
	typeAeiTime onSys2;
	uint32_t difTime;
	int aeiDir;

	axleDetProc();
	void construct(uint32_t _digSys1, uint32_t _digSys2, int _aeiDir, uint32_t _format);
	void update(digInput * dig);
	void calcTime();

};

#endif /* AXLEDETPROCESS_H_ */
