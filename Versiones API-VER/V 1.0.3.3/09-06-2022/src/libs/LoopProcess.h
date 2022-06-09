/*
 * LoopProcess.h
 *
 *  Created on: 16 april 2021
 *      Author: Integra MGZ
*/

#ifndef LIBS_LOOPPROCESS_H_
#define LIBS_LOOPPROCESS_H_

#include <basictypes.h>
#include <sys/_timeval.h>

enum loopState {stActive, stEndCarDet, stInactive};
enum loopType {loopSingle, loopDouble};

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	//void printInputs();

#ifdef __cplusplus
}
#endif /* __cplusplus */

/***************************************************** Class loopDet **********************************************
*/
class loopDet {
public:
	loopState state;
	loopType type;
	uint32_t digInC;
	uint32_t digInI;
	uint32_t digInO;
	uint32_t fieldStInI;
	uint32_t fieldStInO;
	//uint32_t maxSecsOn;
	uint32_t minSecsOff;
	//volatile bool flagTON;
	volatile bool flagTOFF;
	//volatile time_t TON;
	volatile time_t TOFF;

	loopDet();
	void construct(uint32_t _digInputC_I, uint32_t _digInputO, loopType _type, uint32_t _minSecsOff);
	void updateType(loopType _type);
	void update(digInput * dig);
	void timer();

};

#endif /* LIBS_LOOPPROCESS_H_ */
