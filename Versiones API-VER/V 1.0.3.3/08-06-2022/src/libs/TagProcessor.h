/*
 * tagProcessor.h
 *
 *  Created on: 5 abr 2021
 *      Author: Integra Fredy
 */

#ifndef TAGPROCESSOR_H_
#define TAGPROCESSOR_H_

#include <nbstring.h>
#include "libs/Rtc_Controller.h"
#include <libs/TagUtilities.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

//<body>
	void printAarTag(typeAarDec *tagPrn);
	void saveHexTag(char * tag, typeAeiTime * t, uint32_t speed, int direction);
	void getProcessedTag();
	void initProcessor();
	void startProcessor();
	void addStartEndTag(bool start);
	int getFifoCarsSize();
	void moveFifoCarsReader();

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* TAGPROCESSOR_H_ */
