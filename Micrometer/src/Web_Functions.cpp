/*
 * Web_Functions.cpp
 *
 *  Created on: 4 oct 2021
 *      Author: Integra Fredy
 */
#include <http.h>
#include <iosys.h>
#include <httppost.h>
#include <fdprintf.h>
#include <config_obj.h>
#include <string.h>
#include "headers/Web_Functions.h"
#include "headers/Configurations.h"
#include "headers/Business.h"

Configurations configs;

NBString action;
void FormWebProcess(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			if (strcmp(pName, "action") == 0) {
				action = (NBString)pValue;
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
			ProcessState state = noProcess;
			if(strcmp(action.c_str(),"open")==0){
				state = openProcess;
			}else if(strcmp(action.c_str(),"close")==0){
				state = closeProcess;
			}else if(strcmp(action.c_str(),"test")==0){
				state = startedTest;
			}else if(strcmp(action.c_str(),"manual")==0){
				state = manualTest;
			}else if(strcmp(action.c_str(),"stopManual")==0){
				state = stopManual;
			}else if(strcmp(action.c_str(),"changeMode")==0){
				state = modeChanged;
			}
			ProcessState rState = webCommands(state);
			if(rState != startedTest){
				if(rState == modeChanged){
					writestring(sock, "200");
				}else if(rState == badMode){
					if(state == modeChanged){
						writestring(sock, "No se pudo cambiar el modo, espere o finalice las pruebas");
					}else{
						writestring(sock, "No se pudo iniciar la prueba, cambie el modo de operación");
					}
				}else{
					char info[100];
					sprintf(info, "Actualmente las fases estan en Estado de %s\r\n", ProcessInfo[state]);
					writestring(sock, info);
				}
			}else{
				writestring(sock, "200");
			}
		}
		break;
	}
}

void FormPostGeneral(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			break;
		case eVariable:
			//operator
			if (strcmp(pName, "doperator") == 0) {
				strcpy(configs.dOperator, pValue);
			}else if (strcmp(pName, "dsubstation") == 0) {
				strcpy(configs.dSubStation, pValue);
			} else if (strcmp(pName, "ddevice") == 0) {
				strcpy(configs.dDevice, pValue);
			} else if (strcmp(pName, "dserialnum") == 0) {
				strcpy(configs.dSerialNum, pValue);
			} else if (strcmp(pName, "dmake") == 0) {
				strcpy(configs.dMaker, pValue);
			} else if (strcmp(pName, "dcapacity") == 0) {
				strcpy(configs.dCapacity, pValue);
			} else if (strcmp(pName, "dtype") == 0) {
				strcpy(configs.dType, pValue);
			} else if (strcmp(pName, "dtension") == 0) {
				strcpy(configs.dTension, pValue);
			} else if (strcmp(pName, "dreportnum") == 0) {
				configs.dReportNum = atoi(pValue);
			} else if (strcmp(pName, "ddivision") == 0) {
				strcpy(configs.dDivision, pValue);
			} else if (strcmp(pName, "dzone") == 0) {
				strcpy(configs.dZone, pValue);
			} else if (strcmp(pName, "ddate") == 0) {
				strcpy(configs.dDate, pValue);
			} else if (strcmp(pName, "ddatelast") == 0) {
				strcpy(configs.dDateLastTest, pValue);
			} else if (strcmp(pName, "tinitialState") == 0) {
				configs.tInitialState = atoi(pValue);
			} else if (strcmp(pName, "tcycleNumber") == 0) {
				configs.tCycleNumber = atoi(pValue);
			} else if (strcmp(pName, "tcycleWaitTime") == 0) {
				configs.tCycleWaitTime = atoi(pValue);
			} else if (strcmp(pName, "tcmdWaitTime") == 0) {
				configs.tCmdWaitTime = atoi(pValue);
			} else if (strcmp(pName, "tClosePulseTime") == 0) {
				configs.tClosePulseTime = atoi(pValue);
			} else if (strcmp(pName, "tOpenPulseTime") == 0) {
				configs.tOpenPulseTime = atoi(pValue);
			} else if (strcmp(pName, "tphasesWaitTime") == 0) {
				configs.tPhasesWaitTime = atoi(pValue);
			}else if (strcmp(pName, "tmaxCloseDiference") == 0) {
				configs.tMaxDiffClose = atoi(pValue);
			}else if (strcmp(pName, "tmaxOpenDiference") == 0) {
				configs.tMaxDiffOpen = atoi(pValue);
			}else if (strcmp(pName, "tmaxTimeAfterPulse") == 0) {
				configs.tMaxTimeAfterPulse = atoi(pValue);
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
				setNewConfiguration(&configs);
				RedirectResponse(sock, "general_config.html");
			}
			break;
	}
}
int day=0,month=0,year=0,wday = 0,sec=0,min=0,hour=0;
void FormPostSync(int sock, PostEvents event, const char * pName, const char * pValue) {
	switch (event) {
		case eStartingPost:
			day=0,month=0,year=0,sec=0,min=0,hour=0,wday = 0;
			break;
		case eVariable:
			if (strcmp(pName, "date") == 0) {
				day = atoi(pValue);
			}else if (strcmp(pName, "month") == 0) {
				month = atoi(pValue);
			}else if (strcmp(pName, "year") == 0) {
				year = atoi(pValue);
			}else if (strcmp(pName, "hour") == 0) {
				hour = atoi(pValue);
			}else if (strcmp(pName, "minutes") == 0) {
				min = atoi(pValue);
			}else if (strcmp(pName, "seconds") == 0) {
				sec = atoi(pValue);
			}else if (strcmp(pName, "wday") == 0) {
				wday = atoi(pValue);
			}
			break;
		case eFile:
			break;
		case eEndOfPost: {
				rtcSetManual(sec,min,hour,month, day, wday,year);
				RedirectResponse(sock, "index.html");
			}
			break;
	}
}

#define HTTP_BUFFER_SIZE (32 * 1024)   // Make a 32KB BUFFER
static char HTTP_buffer[HTTP_BUFFER_SIZE] __attribute__((aligned(16)));

void SendFragment(int sock, F_FILE *f, long len)
{
    int lread = 0;
    while (lread < len)
    {
        int ltoread = len - lread;
        int lr;

        if (ltoread > HTTP_BUFFER_SIZE) { ltoread = HTTP_BUFFER_SIZE; }

        lr = f_read(HTTP_buffer, 1, HTTP_BUFFER_SIZE, f);

        if (lr == 0) { return; }

        lread += lr;
        writeall(sock, HTTP_buffer, lr);
    }
}

int SendEFFSCustomHeaderResponse(int sock, char *fType, char *pName)
{
    char mime_type[64];
    bool found = false;
        // no MIME.txt found or extension type not found, fall back to hard-coded list
	found = true;   // Set to true. Revert to false if not found in default else.
	if (strcasecmp(fType, "jpg") == 0) {
		sniprintf(mime_type, 64, "image/jpeg");
	}
	else if (strcasecmp(fType, "gif") == 0)
	{
		sniprintf(mime_type, 64, "image/gif");
	}
	else if (strcasecmp(fType, "htm") == 0)
	{
		sniprintf(mime_type, 64, "text/html");
	}
	else if (strcasecmp(fType, "html") == 0)
	{
		sniprintf(mime_type, 64, "text/html");
	}
	else if (strcasecmp(fType, "xml") == 0)
	{
		sniprintf(mime_type, 64, "text/xml");
	}
	else if (strcasecmp(fType, "css") == 0)
	{
		sniprintf(mime_type, 64, "text/css");
	}
	else if (strcasecmp(fType, "mp4") == 0)
	{
		sniprintf(mime_type, 64, "video/mp4");
	}else if (strcasecmp(fType, "csv") == 0)
	{
		sniprintf(mime_type, 64, "text/csv");
	}
	else
	{
		found = false;
	}

    char buffer[255];
    if (found)
    {
        sniprintf(buffer, 255,
                  "HTTP/1.0 200 OK\r\n"
                  "Pragma: no-cache\r\n"
                  "MIME-version: 1.0\r\n"
                  "Content-Type: %s\r\n"
        		  "Content-disposition: attachment;filename=%s\r\n\r\n",
                  mime_type,pName);
    }
    else
    {
        sniprintf(buffer, 255, "HTTP/1.0 200 OK\r\nPragma: no-cache\r\n\r\n");
    }
    int bytes = writestring(sock, buffer);
    return bytes;
}

int HandleGet(int sock, HTTP_Request &pr)
{
    char ext_buffer[10];
    char pName[MAX_FILE_NAME];
    getMicrometerFileData(pName, ext_buffer);
#if (defined(USE_MMC))
    f_chdrive(MMC_DRV_NUM);
#endif

	F_FILE *f = f_open(pName, "r");
	if (f != nullptr)
	{
		long len = f_filelength(pName);
		SendEFFSCustomHeaderResponse(sock, ext_buffer,pName);
		SendFragment(sock, f, len);
		f_close(f);
		close(sock);
		//iprintf(" File sent to browser\r\n");
		return 1;
	}
	RedirectResponse(sock, "test.html");

    return 0;
}

CallBackFunctionPageHandler gHandleGet("Resultados_de_prueba*", HandleGet, tGet, 0, true);
HtmlPostVariableListCallback filePostFormGeneral("formGeneral*", FormPostGeneral);
HtmlPostVariableListCallback webProcessPost("formProcess*", FormWebProcess);

//HtmlPostVariableListCallback webSyncPost("formSync*", FormPostSync);
