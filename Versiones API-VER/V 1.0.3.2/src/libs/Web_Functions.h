/*
 * Web_Functions.h
 *
 *  Created on: 21 abr 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_WEB_FUNCTIONS_H_
#define LIBS_WEB_FUNCTIONS_H_
#include <nbstring.h>


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	struct SargasConfig{
		int SargasQty;
		NBString ip_r;
		NBString ip_l;
		int antQtyR;
		int antQtyL;
	};

	struct UdpConfig{
		int LogChoice;
		NBString ip;
		int port;
	};

	struct ServerConfig{
		NBString url;
	};

	struct inputStruct{
		int Sys1;
		int Sys1p;
		int Sys1n;
		int Sys2;
		int Sys2p;
		int Sys1np;
		int Sys2n;
		int Sys2np;
		int LoopC_I;
		int LoopO;
		int DcOk1;
		int DcOk2;
		int AcOk1;
		int Bat1;
		int AcOk2;
		int Bat2;
		int Door;
		int Free;
	};

	struct EthernetConfig{
		NBString mode;
		NBString ip;
		NBString mask;
		NBString gate;
		NBString dns1;
	};

	struct GeneralConfig{
		NBString AeiID;
		int LoopType;
		int LoopTimeOn;
		int LoopTimeOff;
		bool LoopRfidOnOff = false;
		bool AeiDir;
		bool NtpSyncOnOff;
		NBString NtpServer;
		int NtpSyncTime;
		NBString PhoneNumber;
		bool BattTestOnOff = false;
		NBString BattInstallDate;
		int BattTestPeriod;
		int BattTestHour;
		int TempAlarmSet;

		int MainADSignal;
		bool DoubleAxlesDetector = false;
		int DoubleADSignal;
		bool MirrorAxlesDetector = false;
		int MirrorADSignal;
		bool ReverseDoubleAD = false;
		bool ReverseMirrorAD = false;
	};

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* LIBS_WEB_FUNCTIONS_H_ */
