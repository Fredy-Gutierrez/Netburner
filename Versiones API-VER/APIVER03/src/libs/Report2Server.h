/*
 * Report2Server.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_REPORT2SERVER_H_
#define LIBS_REPORT2SERVER_H_
//<include>

#include <json_lexer.h>
#include <nbstring.h>

//</include>
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	void sendUdpPacket(NBString buffer);
	ParsedJsonDataSet SendKeepAlive();
	void SendMirrorData(char * msg[][2]);
	bool getSending();
	void Report(NBString tags);

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_REPORT2SERVER_H_ */
