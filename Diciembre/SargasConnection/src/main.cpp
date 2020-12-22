#include <arp.h>
#include <predef.h>
#include <netinterface.h>
#include <serial.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <startnet.h>
#include <nbrtos.h>
#include <iosys.h>
#include <utils.h>
#include <tcp.h>
#include <string.h>
#include <fdprintf.h>
#include <http.h>
#include <httppost.h>
#include <webclient/http_funcs.h>
#include <ip.h>
#include <ipshow.h>
#include <nettypes.h>
#include <buffers.h>
#include <json_lexer.h>
#include <nbstring.h>
#include <init.h>
#include <stopwatch.h>
#include <system.h>

const char * AppName="SargasConnection";

StopWatch myStopwatch;


uint32_t   TcpTaskStack[USER_TASK_STK_SIZE];
int fd,sock;
int    gDestPort;
IPADDR gDestIp;


#define RX_BUFSIZE (4096)
char RXBuffer[RX_BUFSIZE];


#define MAX_MSG_SIZE 1024
char fecha[MAX_MSG_SIZE];
char hora[MAX_MSG_SIZE];


static config_string gIp{appdata, "", "SIp"};
config_int gPort{appdata, 0, "Port"};


const unsigned char onRf[] = { 0x23, 0x72 , 0x65 , 0x61 , 0x64 , 0x65 , 0x72 , 0x2E , 0x72 , 0x66 , 0x2E , 0x6F, 0x6E, 0x0D };
const unsigned char offRf[]={ 0x23, 0x72 , 0x65 , 0x61 , 0x64 , 0x65 , 0x72 , 0x2E , 0x72 , 0x66 , 0x2E , 0x6F , 0x66, 0x66 , 0x0D };


const char * urlkeepalive ="http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiAlive&milis=1607468625781&userId=Integra";
const char * urlProcessData ="http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiProcessData&milis=1587171056763&userId=Integra";
const char * urlMirror ="http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiMirrorData&milis=1607468997173&userId=Integra";


/****************************SEND THE JSON DATA THROUGH POST AT A WEB SERVICE********************************/
void SendRxMessage(NBString nbs)
{
	NBString msg;

	msg = (const char*)"#00 INTEGRA 101 00001\n";

	msg += nbs.c_str();

	msg += (const char*)"#99 0000000223";

    ParsedJsonDataSet JsonOutObject;
    ParsedJsonDataSet JsonInObject;

    JsonOutObject.StartBuilding();
    JsonOutObject.Add("action", "aeiCommand");
    JsonOutObject.Add("command", "integraAeiProcessData");
    JsonOutObject.Add("aeiId", "INTEGRA");
    JsonOutObject.Add("rawData", msg.c_str());
    JsonOutObject.Add("userId", "Integra");
    //JsonOutObject.

    JsonOutObject.DoneBuilding();

    bool result = DoJsonPost(urlProcessData, JsonOutObject, JsonInObject, NULL, 10 * TICKS_PER_SECOND);

    if (result)
    {
        iprintf("Integra said [%s]\r\n", JsonInObject.FindFullNameString("info"));
    }
    else
    {
        iprintf("Result failed\r\n");
    }
}

ParsedJsonDataSet SendKeepAlive()
{
    ParsedJsonDataSet JsonInObject;
    bool result = DoGet(urlkeepalive, JsonInObject, 10 * TICKS_PER_SECOND);

    if (result)
    {
        iprintf("Integra said [%s]\r\n", JsonInObject.FindFullNameString("responseCode"));
    }
    else
    {
        iprintf("Result failed\r\n");
    }
    return JsonInObject;
}

void SendMirrorData(char * msg[][2])
{
    ParsedJsonDataSet JsonOutObject;

    ParsedJsonDataSet JsonInObject;

    JsonOutObject.StartBuilding();

    JsonOutObject.Add(msg[0][0], msg[0][1]);

    JsonOutObject.DoneBuilding();

    bool result = DoJsonPost(urlMirror, JsonOutObject, JsonInObject, NULL);

    if (result)
    {

    	//imprime el json completo
    	//JsonInObject.PrintObject(true);

        //Retorna 1 si el objeto existe, 0 si no
    	//printf("%d",JsonInObject.FindObject("info"));

    	printf("Integra said [%s] \r\n", JsonInObject.FindFullNameString("info"));

    	//find the element data01 in object info from json result
    	//JsonInObject.FindFullName("info.data01");
    	//print the val of the element found before
    	//printf("%s",JsonInObject.CurrentString());
    }
    else
    {
        iprintf("Result failed\r\n");
    }
}

void ReportServer(void *pd){

}
/****************************END THE FUNCTIONS TO SEND JSON DATA THROUGH POST AT A WEB SERVICE********************************/


/******************************************TCP FUNCTIONS************************************************/
void TcpTask(void * pd)
{
	while(1){
		IPADDR destIp = gDestIp;
		int destPort = gDestPort;

		iprintf( "Connecting to: %I:%d\r\n", destIp, destPort );

		if (fd > 0) close(fd);

		fd = connect( destIp, destPort, TICKS_PER_SECOND * 5);

		if (fd < 0)
		{
			iprintf("Error: Connection failed, return code: %d\r\n", fd);
		}
		else
		{
			int y = 0, n= 0;
			do
			{
				n = ReadWithTimeout( fd, RXBuffer, RX_BUFSIZE, 5000);
				RXBuffer[n] = '\0';

				iprintf( "Read %d bytes: %s\n", n, RXBuffer );
				if(y != 0 && n>0){
					myStopwatch.Stop();

					unsigned long long elapsedTime = myStopwatch.GetTime();
					printf("Elapsed time: %llu, %e seconds\r\n", elapsedTime, myStopwatch.Convert(elapsedTime) );
					printf("Resolution: %e seconds\r\n", myStopwatch.CountResolution() );

					//SendRxMessage(RXBuffer);
				}

				y = 1;

			} while ( n > 0 );
		}

		iprintf("Sargas Connection close\n");
		OSTimeDly(TICKS_PER_SECOND * 1);
	}
}

void SargasSend(){

	IPADDR destIp = gDestIp;

	int fd2 = connect( destIp, 2711, TICKS_PER_SECOND * 5);

	int n = write( fd2, (char *)&offRf, sizeof(offRf));

	iprintf( "Wrote %d bytes\r\n", n );

	close(fd2);

	fd2 = 0;
}

void SargasOn(){

	IPADDR destIp = gDestIp;

	int fd2 = connect( destIp, 2711, TICKS_PER_SECOND * 5);

	int n = write( fd2, (char *)&onRf, sizeof(onRf));

	iprintf( "Wrote %d bytes\r\n", n );

	close(fd2);

	fd2 = 0;
}
/******************************************END TCP FUNCTIONS************************************************/


/********************************FUNCTIONS SAVE GLOBAL VARIABLES************************************/
void UpdateConfigIp(config_string &confIp, const char * val)
{
    // Display the old value and get a new value from user input
    NBString confName;

    confIp.GetNameValue(confName);

    iprintf("%s old value: %s\r\n", confName.c_str(), confIp.c_str());

    confIp = val;

    iprintf("%s new value: %s\r\n", confName.c_str(), confIp.c_str());

    SaveConfigToStorage();
}

void UpdateConfigPort(config_int &confPort, const char * val)
{
    // Display the old value and get a new value from user input
    NBString confName;
    confPort.GetNameValue(confName);//Ingresa nombre de la variable de configuracion en ConfName
    iprintf("%s old value: %d\r\n", confName.c_str(), int(confPort));

    confPort = atoi(val);//parseInt

    iprintf("%s new value: %d\r\n", confName.c_str(), int(confPort));

    // Now force it to save
    SaveConfigToStorage();
}
/********************************END FUNCTIONS SAVE GLOBAL VARIABLES************************************/


/***************************************************WEBS FUNCTIONS*****************************************************************/
void processPostVariables( const char *pName, const char *pValue )
{
    iprintf("Processing: %s\r\n", pName);

    if( strcmp(pName, "puerto") == 0 )
    {
    	UpdateConfigPort(gPort, pValue);

        gDestPort = (uint16_t)atoi( pValue );

        iprintf( "Destination port set to: %d\r\n", gDestPort );
    }
    else if( strcmp( pName, "ip") == 0 )
    {
        UpdateConfigIp(gIp, pValue);

        iprintf( "DestIpAddr set to: %s\r\n", pValue );
        gDestIp = AsciiToIp( pValue );
    }
    else
    {
        iprintf("Error processing %s\r\n", pName);
    }
}

void FormPostCallBack(int sock, PostEvents event, const char * pName, const char * pValue)
{
    // Received a call back with an event, check for event type
    switch (event)
    {
    case eStartingPost:     // Called at the beginning of the post before any data is sent
    	if (fd > 0) close(fd);
    	break;

    case eVariable:     // Called once for each variable in the form
    	processPostVariables(pName, pValue);
        break;
    //Called back with a file handle if the post had a file
    case eFile:
        break; //No file type here so we do nothing

    // Called back when the post is complete. You should send your response here.
    case eEndOfPost:
        {
            RedirectResponse(sock, "index.html");
        }
        break;
    }
}

void WebAlive(int sock, PCSTR url)
{
	ParsedJsonDataSet jsonIn = SendKeepAlive();
	fdprintf(sock, "Code: %s, Server date: %s", jsonIn.FindFullNameString("responseCode"),jsonIn.FindFullNameString("serverDate"));
}
/***************************************************END WEBS FUNCTIONS*****************************************************************/


void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait up to 5 seconds for active network activity 
    StartHttp(19000);

    iprintf("Application %s started\n",AppName );

    showIpAddresses();

    ParsedJsonDataSet JsonOutObject;
    /*{
     * 	"data1": "val1"
     *}*/
    JsonOutObject.StartBuilding();//{
    JsonOutObject.Add("data1","val1");//"data1": "val1"

    /*{
     * 	"data1": "val1",
     * 	"data2":[
     * 		"data2": "val2"
     * 	]
     *}*/
    //JsonOutObject.AddArrayStart("data2");//"data2":[
    //JsonOutObject.Add("data2", "val2");//"data2":"val2"
    //JsonOutObject.EndArray();//]

    /*{
     * 	"data1": "val1",
     * 	"data2":[
     * 		"data2": "val2"
     * 	],
     * 	"data3":{
     * 		"data3": "val3"
     * 	}
     *}*/
    JsonOutObject.AddObjectStart("data2");//data3:{
    JsonOutObject.Add("data2", "val2");//"data3":"val3"
    JsonOutObject.EndObject();//}

    JsonOutObject.DoneBuilding();//}


    //find a val in a object´s element
    //JsonOutObject.FindFullName("data3.data3");
    //print the val of the element found before
    //printf("%s",JsonOutObject.CurrentString());

    //JsonOutObject.PrintObject(true);


    gDestPort = int(gPort);
    gDestIp = AsciiToIp(gIp.c_str());


    OSTaskCreatewName( TcpTask,
		(void  *)NULL,
		&TcpTaskStack[USER_TASK_STK_SIZE] ,
		TcpTaskStack,
		(MAIN_PRIO -1),   // higher priority than UserMain
		"TCP" );

    OSSimpleTaskCreatewName(ReportServer, MAIN_PRIO -2 , "Report");

    NBString pruebaLectura;
    while (1)
    {
    	char opcion;
    	scanf("%c", &opcion);
    	iprintf("\n");


    	switch (opcion) {
    		case '1':
    			SendKeepAlive();
    			break;
    		case '2':
    			char * msg[2][2];
    			/*
    			 * "data 01": "value 01",
    			*/
    			msg[0][0]= "data01";
    			msg[0][1]= "value 01";

    			SendMirrorData(msg);
    			break;
    		case '3':
    			pruebaLectura += (const char *)"#19 TEST 908765 1 &19:03:15.71 03/31/20%00-1-01-0\n#19 TEST 908765 1 &19:03:15.94 03/31/20%00-0-01-0\n";
    			SendRxMessage(pruebaLectura);
    			break;
    		case '4':
    			SargasSend();
    			break;
    		case '5':
    			SargasOn();
    			myStopwatch.Start();
    			break;
    		case '6':
    		    break;
    		case '7':
    		    break;
    		default:
    			iprintf("La opcion marcada es incorrecta\n");
    			break;
    	}
    	OSTimeDly(TICKS_PER_SECOND * 1);
    }

}

HtmlPostVariableListCallback postForm1("form1*", FormPostCallBack);
