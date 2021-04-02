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
#include "libs/Configurations.h"
#include "libs/Sargas_Connection.h"


/*********************************************FUNCTIONS THAT THAT PROCESS THE DATA RECEIVED FOR SARGAS***********************************************/
void processPostVariables(const char *pName, const char *pValue) {
	iprintf("Processing: %s\r\n", pName);

	if (strcmp(pName, "puerto") == 0) {
		//comm.UpdateConfigPort(pValue);

	} else if (strcmp(pName, "ip") == 0) {
		//comm.UpdateConfigIp(pValue);
		setSargasIp(pValue);
		closeFd();
	} else {
		iprintf("Error processing %s\r\n", pName);
	}
}

//FUNCTION THAT RECEIVES THE WEB'S DATA BY SARGAS CONF/
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

		RedirectResponse(sock, "sargas_config.html");
	}
		break;
	}
}


/*********************************************FUNCTION THAT RECEIVES THE WEB SERVER DATA***********************************************/
void FormPostServer(int sock, PostEvents event, const char * pName, const char * pValue) {
	// Received a call back with an event, check for event type
	switch (event) {
	case eStartingPost: // Called at the beginning of the post before any data is sent

		break;

	case eVariable:     // Called once for each variable in the form
		if (strcmp(pName, "server_address") == 0) {
			setServerUrl(pValue);
		} else {
			iprintf("Error processing %s\r\n", pName);
		}
		break;
		//Called back with a file handle if the post had a file
	case eFile:
		break; //No file type here so we do nothing

		// Called back when the post is complete. You should send your response here.
	case eEndOfPost: {
		//comm.closeConection();

		RedirectResponse(sock, "server_config.html");
	}
		break;
	}
}


/*********************************************FUNCTION THAT RECEIVES UDP DATA***********************************************/
void FormPostUdp(int sock, PostEvents event, const char * pName, const char * pValue) {
	// Received a call back with an event, check for event type
	switch (event) {
	case eStartingPost: // Called at the beginning of the post before any data is sent

		break;

	case eVariable:     // Called once for each variable in the form
		if (strcmp(pName, "udpPort") == 0) {
			setUdpPort( atoi(pValue) );
		} else if (strcmp(pName, "udpIp") == 0) {
			setUdpIp(pValue);
		} else {
			iprintf("Error processing %s\r\n", pName);
		}
		break;
		//Called back with a file handle if the post had a file
	case eFile:
		break; //No file type here so we do nothing

		// Called back when the post is complete. You should send your response here.
	case eEndOfPost: {
		//comm.closeConection();

		RedirectResponse(sock, "server_config.html");
	}
		break;
	}
}

//Register a call back for the web page...
HtmlPostVariableListCallback filePostForm("form1*", FormPostCallBack);
//formServerConfig
HtmlPostVariableListCallback filePostFormServer("formServerConfig*", FormPostServer);

HtmlPostVariableListCallback filePostFormUdp("formUdpConfig*", FormPostUdp);





