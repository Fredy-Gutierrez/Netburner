/*
 * Web_Functions.cpp
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */
#include <http.h>
#include <iosys.h>
#include <httppost.h>
#include <fdprintf.h>
#include <config_obj.h>
#include <string.h>
#include <libs/Configurations.h>
#include "libs/Sargas_Connection.h"

void setNew() {

}

void processPostVariables(const char *pName, const char *pValue) {
	iprintf("Processing: %s\r\n", pName);

	if (strcmp(pName, "puerto") == 0) {
		//comm.UpdateConfigPort(pValue);

	} else if (strcmp(pName, "ip") == 0) {
		//comm.UpdateConfigIp(pValue);
		setSargasIp(pValue);
		closeFd();

		iprintf("DestIpAddr set to: %s\r\n", pValue);
	} else {
		iprintf("Error processing %s\r\n", pName);
	}
}

void FormPostCallBack(int sock, PostEvents event, const char * pName,
		const char * pValue) {
	// Received a call back with an event, check for event type
	switch (event) {
	case eStartingPost: // Called at the beginning of the post before any data is sent

		break;

	case eVariable:     // Called once for each variable in the form
		processPostVariables(pName, pValue);
		break;
		//Called back with a file handle if the post had a file
	case eFile:
		break; //No file type here so we do nothing

		// Called back when the post is complete. You should send your response here.
	case eEndOfPost: {
		//comm.closeConection();

		RedirectResponse(sock, "index.html");
	}
		break;
	}
}

//Register a call back for the web page...
HtmlPostVariableListCallback filePostForm("form1*", FormPostCallBack);

