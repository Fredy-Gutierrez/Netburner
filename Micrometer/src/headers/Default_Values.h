/*
 * Default_Values.h
 *
 *  Created on: 4 mar 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_DEFAULT_VALUES_H_
#define LIBS_DEFAULT_VALUES_H_

#include <constants.h>

/********************************APP VALS*******************************/
#define APPVERSION "0.0.0.2"

#define DEFAULT_NTP_SERVER_NAME "mx.pool.ntp.org"//pool.ntp.org
#define DEFAULT_NTP_RETRIES 3

#define MICROMETER_PRIO	MAIN_PRIO - 1
#define FTP_PRIO	MAIN_PRIO + 3
#define TERMINAL_PRIO	MAIN_PRIO + 4
#define NTP_TASK_PRIO MAIN_PRIO+5

#define FIFOBUFFERSIZE 300
#define MAXBUFFERSIZE 1024

#define ALIASDIGITALSIZE 10

#define CLOSEDIGITALOUT	0			//array position output number for close command
#define OPENDIGITALOUT	1			//array position output number for open command
#define CLOSEDIGITALINPOSITION	3	//array position input number for close command
#define OPENDIGITALINPOSITION	4	//array position input number for open command
#define CMDDIGITALSIN	2			//this is the number of comands inputs, in this case there are 2 inputs for the two cmds (open/close)

#define HMAKERDEFAULT "INTEGRA"
#define HTYPEDEFAULT "HIID 3F"
#define HSERIALDEFAULT "UTRHV1-2010210001"

#define DOPERATORDEFAULT "UNKNOWN"
#define DSUBSTATIONDEFAULT "UNKNOWN"
#define DDEVICEDEFAULT "UNKNOWN"
#define DSERIALDEFAULT "XXXX-XXXX-XXXX-XXXX"
#define DMAKERDEFAULT "UNKNOWN"
#define DCAPACITYDEFAULT "UNKNOWN"
#define DTYPEDEFAULT "UNKNOWN"
#define DTENSIONDEFAULT "UNKNOWN"
#define DREPORTNUMDEFAULT 0
#define DDIVISIONDEFAULT "UNKNOWN"
#define DZONEDEFAULT "UNKNOWN"
#define DDATEDEFAULT "2021-01-01"
#define DDATELASTTESTDEFAULT "2021-01-01"

#define TINITIALSTATEDEFAULT 1
#define TCYCLENUMBERDEFAULT 3
#define TCYCLEWAITTIMEDEFAULT 30 //wait time after a cycle
#define TCMDWAITTIMEDEFAULT 25 //wait time after a cmd
#define TCLOSEPULSETIMEDEFAULT 15 //pulse cmd duration in ms
#define TOPENPULSETIMEDEFAULT 15 //pulse cmd duration in ms
#define TPHASESWAITTIMEDEFAULT 1000 //pulse cmd duration in ms
#define TMAXDIFFCLOSEDEFAULT 4 //MAX close diff duration in ms
#define TMAXDIFFOPENDEFAULT 4 //MAX OPEN diff duration in ms
#define TMAXTIMEAFTERPULSEDEFAULT 100 //MAX TIME duration after send pulse in ms

#endif /* LIBS_DEFAULT_VALUES_H_ */
