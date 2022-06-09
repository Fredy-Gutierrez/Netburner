/*
 * Train.h
 *
 *  Created on: 5 jun 2021
 *      Author: mgz
 */

#ifndef LIBS_TRAIN_H_
#define LIBS_TRAIN_H_

#include <basictypes.h>
#include "libs/TagUtilities.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

	uint32_t CalcTrainSize(typeTrain * train);
	int getTrainCounter();
	typeTrain * getTrain();
	void getAemTrainToReport(typeT94AemPost * segAemPost);
	void getEocTrainToReport(typeT94EocPost * segEocPost);
	void initTrain();
	//void freeTrain(typeTrain * train);
	void initTrainAdmin();
	int getTrainRreAxlesQty();
	int getTrainRreQty();
	void DeleteLastRREFromTrain(bool subsCounters = true);
	void addRreToTrain(typeT94Rre * segRreToSave, bool addCounters = true);
	void closeTrainToSave(typeT94Aem * segAem);
	void fillEot(typeAarDec * tag, int * seqNumber);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBS_TRAIN_H_ */
