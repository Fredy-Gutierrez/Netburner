/*
 * Business.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef BUSINESS_H_
#define BUSINESS_H_

enum MainService { infoStartCarDet, infoEndCarDet, infoCarStopOver, repDigChg, repAxle, infoGeneral, turnOnAnt, turnOffAnt, noService };

#include "headers/Configurations.h"
#include "headers/OSTime.h"
#include "headers/MicrometerProcess.h"
#include "FatLib/FileSystemUtils.h"
#include "headers/IO_Interface.h"
#include "headers/Default_Values.h"
#include "headers/I2C_Controller.h"
#include "headers/Rtc_Controller.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>
	void getWebResults(Fifo * webData);
	ProcessState webCommands(ProcessState webState);
	ProcessState getProcessState(bool update);
	ProcessState getMicrometerState();
	HourMeterMode getMicrometerMode();
	void attnWaitTime();
	void initBusiness();
	//void attendBusiness();
	//void askForService(uint32_t format, void * obj, bool saveCmd);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* BUSINESS_H_ */
