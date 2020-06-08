#include <system.h>
#include <predef.h>
#include <stdio.h>
#include <ctype.h>
#include <tcp.h>
#include <nbrtos.h>
#include <iosys.h>
#include <fdprintf.h>
#include <serial.h>
#include <startnet.h>
#include <constants.h>
#include <pinconstant.h>
#include <pins.h>
#include <quadspi.h>
#include <stddef.h>
#include <stopwatch.h>
#include <nbtime.h>      // Include for NTP functions
#include <time.h>
#include <init.h>
#include <dhcpclient.h>
#include <smarttrap.h>
#include <taskmon.h>
#include <dhcpd.h>
#include <wifi/wifi.h>
#include <dns.h>
#include <ipshow.h>
#include <timezones.h>
#include <buffers.h>
#include <json_lexer.h>
#include <webclient/http_funcs.h>
#include <netinterface.h>
#if (defined IPV6)
#include <ipv6/ipv6_interface.h>
#endif

extern "C"
{
	void UserMain(void * pd);
}

const char * AppName="IntegraID";

OS_SEM MySemaphore;   // Create semaphore

/**********************************DECLARACIONES DEVARIABLES GLOBALES PARA USO***************************************/
#define TCP_LISTEN_PORT 4444   				// PUERTO TCP
#define RX_BUFSIZE (4096)      				// CANTIDAD DEL BUFFER DE RECEPCION Y ENVIO VIA TCP
#define Conn P1				   				// VARIABLE DE CONTROL DE LOS PINS
#define PinOutGpioFn PinIO::PIN_FN_OUT		// VARIABLE DE CONTROL DE SALIDA DE DATOS VIA PIN
#define PinInputGpioFn PinIO::PIN_FN_IN		// VARIABLE DE CONTROL DE ENTRADA DE DATOS VIA PIN
#define xstr(x) str(x)						//
#define str(x) #x							//
#define NTP_SERVER_NAME "pool.ntp.org"      // SERVIDOR NTP DE OBTENCION DE HORA GLOBAL

/**************************************VARIABLES DE CONTROL DE FECHA Y HORA*****************************************/
void  tzsetchar(char * tzenv);
const char *dow_str[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT", "\0" };
int dia,mes,anno,hora,minuto,segundo;
// Allocate task stack for hora task
uint32_t   getHoraStack[USER_TASK_STK_SIZE];

/***********************************VARIABLES DE CONTROL DE LOS PINS Y ESTADOS***************************************/
int pins[10]={20,21,22,23,24,25,26,27,28,29};
bool estadoanterior = 0;
bool estadoactual = 0;
bool estadoreporte = 0;
int filtro = 0;

/**********************************VARIABLES DE CONTROL DE PUERTOS TCP Y SERIAL**************************************/
char RXBuffer[RX_BUFSIZE];
char * msg="";
char * msgserial="";
// Allocate task stack for TCP listen task
uint32_t   TcpServerTaskStack[USER_TASK_STK_SIZE];
uint32_t   SerialServerTaskStack[USER_TASK_STK_SIZE];




/*************************************INICIALIZACION DEL CRONOMETRO**************************************************/
StopWatch myStopwatch;


/*************************************INICIALIZACION DEL MODULO WIFI**************************************************/
NB::Wifi *pNBWifiObject = nullptr;
int ifnumWifi = 0;

//extern DHCP::Server MyServer;
//void PrintDhcpClients( DHCP::Server &server );


/*************************OBTENCION DE LOS VALORES DE LA HORA*****************************************/
void PrintTimeStruct(struct tm &bt )
{
	dia = bt.tm_mday;
	mes = bt.tm_mon + 1;
	anno = bt.tm_year + 1900;
    hora = bt.tm_hour;
    minuto = bt.tm_min;
    segundo = bt.tm_sec;
}
// Format strings for various time zones. Many more in timezones.h and timezones.cpp
const char tzsetFormatString[][120] = {
        {"EST5EDT4,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // eastern
        {"CST6CDT5,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // central
        {"MST7MDT6,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // mountain
        {"PST8PDT7,M3.2.0/01:00:00,M11.1.0/02:00:00"},    // pacific
        {"UTC0"},                                         // UTC Time
		{""}
};

void getHora(void * pd){
	while(1)
	{
		OSTimeDly(TICKS_PER_SECOND * 1);
		time_t tv = time(NULL);
		struct tm tm_struct;
		int i = 0;
		while (tzsetFormatString[i][0] != '\0' )
		{
			tzsetchar((char*) tzsetFormatString[i]);
		    localtime_r(&tv, &tm_struct);
		    if(i==4){
		      PrintTimeStruct(tm_struct);
		    }
		    i++;
		}
	}

}
/****************************FIN DE LA OBTENCION DE LOS VALORES DE LA HORA******************************************/

/****************************ENVIO DE DATOS JSON MEDIANTE POST  A UN WEB SERVICE********************************/
const char * urlpost ="http://harbestm.com/olstorHtmWs/operation/AeiWs.aspx?action=aeiCommand&aeiId=INTEGRA&command=integraAeiProcessData&milis=1587171056763";
void SendRxMessage(char * msj)
{
    ParsedJsonDataSet JsonOutObject;
    ParsedJsonDataSet JsonInObject;

    JsonOutObject.StartBuilding();
    JsonOutObject.Add("action", "aeiCommand");
    JsonOutObject.Add("command", "integraAeiProcessData");
    JsonOutObject.Add("aeiId", "INTEGRA");
    JsonOutObject.Add("rawData", msj);
    JsonOutObject.Add("userId", "ESQ");
    JsonOutObject.DoneBuilding();

    //OutBoundJson.PrintObject(true);

    bool result = DoJsonPost(urlpost, JsonOutObject, JsonInObject, NULL, 10 * TICKS_PER_SECOND);

    if (result)
    {
        iprintf("Integra said [%s]\r\n", JsonInObject.FindFullNameString("info"));
    }
    else
    {
        iprintf("Result failed\r\n");
    }
}

/******************************************FIN DE FUNCIONES POST************************************************/



/************************************INICIO DE LA TAREA TcpServerTask************************************************/
void TcpServerTask(void * pd)
{
    int listenPort = (int) pd;

    // Set up the listening TCP socket
    int fdListen = listen(INADDR_ANY, listenPort, 5);

    if (fdListen > 0)
    {
        IPADDR      clientAddress;
        uint16_t    clientPort;

        while(1)
        {
            /* The accept() function will block until a TCP client requests a connection. Once a client
             * connection is accepting, the file descriptor fdAccept is used to read/write to it.
             */
        	iprintf( "Waiting for connection on port %d...\n", listenPort );
            int32_t fdAccept = accept(fdListen, &clientAddress, &clientPort, 0);
            iprintf("Connected to: %I\r\n", GetSocketRemoteAddr(fdAccept));

            writestring(fdAccept, "Welcome to the NetBurner TCP Server\r\n");
            fdprintf(fdAccept, "You are connected to IP Address %I:%d\r\n", GetSocketRemoteAddr(fdAccept), GetSocketRemotePort(fdAccept) );

            while (fdAccept > 0)
            {
                /* Loop while connection is valid. The read() function will return 0 or a negative number if the
                 * client closes the connection, so we test the return value in the loop. Note: you can also use
                 * ReadWithTimout() in place of read to enable the connection to terminate after a period of inactivity.
                */
                int n = 0;
                do
                {
                    n = read( fdAccept, RXBuffer, RX_BUFSIZE );
                    RXBuffer[n] = '\0';
                    iprintf( "Read %d bytes: %s\n", n, RXBuffer );
                    SendRxMessage(RXBuffer);
                } while ( n > 0 );

                iprintf("Closing client connection: %I\r\n", GetSocketRemoteAddr(fdAccept) );
                close(fdAccept);
                fdAccept = 0;
            }
           //MySemaphore.Post();
        } // while(1)
    } // while listen
}
/***************************************FIN DE LA TAREA TcpServerTask************************************************/



/************************************INICIO DE LA TAREA SerialServerTask************************************************/
void SerialServerTask(void *notUsed)
{
	/**********************************COMANDOS DE INICIO DEL PUERTO SERIAL************************************/
	    SerialClose(0);
	    SerialClose(1);
	    // Open the serial ports....
	    int fd0 = OpenSerial(0, 115200, 1, 8, eParityNone);
	    int fd1 = OpenSerial(1, 115200, 1, 8, eParityNone);

	    // Now write something out both ports
	    writestring(fd1, "Test1");
	    writestring(fd0, "Test0");
	/********************************FIN DE COMANDOS DE INICIO DEL PUERTO SERIAL************************************/

	while(1){
				//MySemaphore.Pend(TICKS_PER_SECOND * 15); //funcion para detener el semaforo hasta el tiempo marcado o hasta que el post lo libere
/****************************OBTENCION DE LOS VALORES DE LOS PUERTOS SERIALES*******************************/
				fd_set read_fds;
				FD_ZERO(&read_fds);
		        FD_SET(fd1, &read_fds);
		        FD_SET(fd0, &read_fds);
		        if (select(FD_SETSIZE, &read_fds, (fd_set *)0, (fd_set *)0, TICKS_PER_SECOND * 5))
		        {
		            if (FD_ISSET(fd1, &read_fds))
		            {
						#ifdef MODM7AE70
										putleds(0x22);
						#endif
		                char buffer[1024];
		                int n = read(fd1, buffer, 1024);
		                write(fd0, buffer, n);
		                msgserial = buffer;
		            }

		            if (FD_ISSET(fd0, &read_fds))
		            {
						#ifdef MODM7AE70
										putleds(0x22);
						#endif
		                char buffer[1024];
		                int n = read(fd0, buffer, 1024);
		                write(fd1, buffer, n);
		                msgserial = buffer;
		            }
		        }
		        else
		        {
		            // WE timed out... nothing to send
		        }
		    	/*************************FIN DE OBTENCION DE LOS VALORES DE LOS PUERTOS SERIALES***************************/
	}
}
/************************************FIN DE LA TAREA SerialServerTask************************************************/



/************************************************FUNCIONES DE IMPRESION WEB*******************************************************/
void WebDestPort(int sock, PCSTR url)
{
    //fdprintf(sock, "%s", RXBuffer);
	fdprintf(sock, "%d", estadoanterior);
}

void WebSerialPort(int sock, PCSTR url)
{
	//fdprintf(sock, "%s", msgserial);
    fdprintf(sock, "%d", estadoactual);
}

void WebReporte(int sock, PCSTR url)
{
    //fdprintf(sock, "%s", msg);
	fdprintf(sock, "%d", estadoreporte);
}

void WebHora(int sock, PCSTR url)
{
    //fdprintf(sock, "%s", msg);
	fdprintf(sock, "%02d/%02d/%d <br> %02d:%02d:%02d - UTC", dia,mes,anno,hora,minuto,segundo);
}
/**************************************************FIN DE FUNCIONES DE IMPRESIONES WEB**********************************************/

static uint8_t RXBufferspi[10000], TXBuffer[10000];

void UserMain(void * pd)
{
    init();
    StartHttp();
    WaitForActiveNetwork();   // Wait up to 5 seconds for active network activity

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());
    //MySemaphore.Init();

        /* IP address of the WiFi AP. This address can be used to access the NetBurner webpage and to update the app */
            IPADDR4 wifiAP_IPAddr = AsciiToIp4("192.168.3.1");
            /* Starting IP address for client DHCP leases. Additional address get incremented starting from this address */
            IPADDR4 startAddr = AsciiToIp4("192.168.3.2");

            iprintf("Initializing Wifi and scanning....\r\n");
            ifnumWifi = WifiInitScanAndShow_SPI();

            if(ifnumWifi > 0)
            {
            	/* Configure the WiFi interface's IP address in preperation for the WiFi access point */
            	/* Ideally, the WiFi interface should have a configured IP/Mask/Gate/DNS before/during the
            	 * Wifi AP starting process.
            	 */
            	InterfaceBlock* pWifiIntf = GetInterfaceBlock(ifnumWifi);
        		pWifiIntf->ip4.cur_addr = wifiAP_IPAddr;
        		pWifiIntf->ip4.cur_mask = IPADDR4(255, 255, 255, 0);
        		pWifiIntf->ip4.cur_gate = IPADDR4::NullIP();
        		pWifiIntf->ip4.cur_dns1 = IPADDR4::NullIP();
            }

            // Start the wifi device in Access Point mode. This allows another device to connect
            // directly to this module. If you do not pass an SSID and password as function parameters,
            // the SSID/password from the config record will be used. These can be set using IPSetup.
            // The full call is:
            // int InitAP_SPI(
            //          const char * SSID       = "",
            //          const char * password   = "", /* Password must be between 8 and 64 chars */
            //          uint8_t      channel    = 6, /* 802.11 channel to setup the AP on */
            //          int irqNum              = 3,
            //          int moduleNum           = 1,
            //          int csNum               = NBWIFI_DEFAULT_CSNUM,
            //          int connectorNum        = NBWIFI_DEFAULT_CONNUM,
            //          int gpioPinNum          = NBWIFI_DEFAULT_PINNUM,
            //          int resetPinNum         = 42
            //          );
            //
            ifnumWifi = InitAP_SPI("NetBurnerAP", "password");

            if(ifnumWifi >= 0 )
            {
            	/* Make sure the wifi interface has an IP address before starting the DHCP server */
        		iprintf("Starting Access Point DHCP server... ");
        		if (AddStandardDHCPServer(theWifiIntf->GetSystemInterfaceNumber(), startAddr))
        			iprintf("Success\r\n");
        		else
        			iprintf("Error: another server exists\r\n");

        		/* Print the WiFi chipset hardware and firmware versions */
                NB::Wifi *pNBWifiObject;
                NB::nbWifiDeviceInfo devInfo;
                pNBWifiObject = NB::Wifi::GetDriverByInterfaceNumber( ifnumWifi );
                pNBWifiObject->GetDeviceInformation( &devInfo );
                iprintf("\r\nHardware Rev: %d.%d\r\n", devInfo.hardwareMajorRev, devInfo.hardwareMinorRev);
                iprintf("Wifi Firmware Rev: %d.%d\r\n\r\n", devInfo.softwareMajorRev, devInfo.softwareMinorRev );

        		iprintf("Configured Wifi IP: ");
        		wifiAP_IPAddr.print();
        		iprintf("\r\n");

        		iprintf("Starting DHCP leases at address: ");
        		startAddr.print();
        		iprintf("\r\n\r\n");
            }
            else
            {
            	iprintf("Error: Failed to initialize wifi\r\n");
            	while(1) { OSTimeDly(TICKS_PER_SECOND); }
            }


    /***************************************INICIO DE HORA*****************************************************/
        BOOL result = FALSE;
            while (!result)
            {
                IPADDR ipAddress = IPADDR::NullIP();

                iprintf("Resolving NTP server name: %s...", NTP_SERVER_NAME);
                int rv = GetHostByName(NTP_SERVER_NAME, &ipAddress, INADDR_ANY, TICKS_PER_SECOND * 10);
                ipAddress.print();  iprintf("\r\n");
                if (rv == DNS_OK)
                {
                    iprintf("Using NTP Server to set system time\r\n");
                    result = SetNTPTime(ipAddress);
                    if (!result)
                    {
                        iprintf("SetNTPTime() failed, waiting 30 seconds to try again\r\n");
                        OSTimeDly(TICKS_PER_SECOND * 30);
                    }
                }
                else
                {
                    iprintf("Name resolution failed, %d, waiting 5 seconds to try again\r\n", rv);
                    OSTimeDly(TICKS_PER_SECOND * 5);
                }
            }
    /****************************************FIN DE OBTENCION DE HORA***************************************************/


    /****************************************EJECUCION DE LA TAREA HORA***********************************************/
            /*OSTaskCreatewName( getHora,
                               (void  *)NULL,
                               &getHoraStack[USER_TASK_STK_SIZE] ,
                               getHoraStack,
                               (MAIN_PRIO - 1),   // higher priority than UserMain
                               "Hora" );*/
    /****************************************EJECUCION DE LA TAREA TCP***********************************************/
            OSTaskCreatewName( TcpServerTask,
                                   (void  *)TCP_LISTEN_PORT,
                                   &TcpServerTaskStack[USER_TASK_STK_SIZE] ,
                                   TcpServerTaskStack,
                                   (MAIN_PRIO -1),   // higher priority than UserMain
                                   "TCP Server" );
    /****************************************EJECUCION DE LA TAREA SERIAL***********************************************/
            /*OSTaskCreatewName( SerialServerTask,
                                   (void  *)NULL,
                                   &SerialServerTaskStack[USER_TASK_STK_SIZE] ,
    							   SerialServerTaskStack,
                                   (MAIN_PRIO + 3),   // higher priority than UserMain
                                   "SERIAL Server" );*/

            OSSimpleTaskCreatewName(getHora, MAIN_PRIO -2 , "getHora");
            //OSSimpleTaskCreatewName(OsFlagTask2, (MAIN_PRIO + 2), "OsFlagTask2");

            myStopwatch.Start();   //INICIO DEL CRONOMETRO DE MEDICION DE EJECUCION


            int PACT = 5;          //VARIABLE DE CONTROL DEL REPORTE DE ESTADO ACTUAL
            bool estados[10];      //ARREGLO DE ESTADOS DE LOS PINES


        	OSTimeDly(TICKS_PER_SECOND * 0.0002);     //INTERRUPCION A 200 MICROSEGUNDOS



        /**********************************OBTENCION DEL VALOR OBTENIDO DEL PIN*********************************************/
        	estados[0] = Conn[pins[0]];
        	estados[1] = Conn[pins[1]];
        	estados[2] = Conn[pins[2]];
        	estados[3] = Conn[pins[3]];
        	estados[4] = Conn[pins[4]];
        	estados[5] = Conn[pins[5]];
        	estados[6] = Conn[pins[6]];
        	estados[7] = Conn[pins[7]];
        	estados[8] = Conn[pins[8]];
        	estados[9] = Conn[pins[9]];
        	/***********************************FIN DE LA OBTENCION DE ESTADOS**********************************************/



        	/***********************************IMPRESION DE ESTADOS DE PINES (PRUEBA)**************************************/
        	int var;
        	for (var = 0; var < 10; ++var) {
        		iprintf("Pin %i: %d \n",pins[var],estados[var]);
        	}
        	/******************************FIN DE IMPRESION DE ESTADOS DE PINES (PRUEBA)************************************/



        	/***********************************INICIALIZANDO ESTADOS A REPORTAR*********************************************/
        	estadoanterior = estadoactual;
        	estadoactual = estados[0];

        	if(estadoactual == estadoanterior){
        		filtro++;
        		if(filtro==PACT){
        			estadoreporte=estadoactual;
        		}else{
        			if(filtro>PACT){
        				filtro = PACT;
        			}
        		}
        	}else{
        		PACT = 0;
        	}
        	/**************************************FIN DE ESTADOS A REPORTAR*********************************************/


        	myStopwatch.Stop();//*************** FIN DEL CRONOMETRO
            unsigned long long elapsedTime = myStopwatch.GetTime();  //************OBTENCION DE LOS VALORES DEL CRONOMETRO
            printf("Elapsed time: %lld, %e seconds\r\n", elapsedTime, myStopwatch.Convert(elapsedTime));//*****IMPRESION Y CONVERSION DE LOS VALORES

            // Configure QuadSPI pins for SPI mode
                P2[48].function(PINP2_48_QCS);      // Chip Select
                P2[45].function(PINP2_45_QSCK);     // CLOCK
                P2[47].function(PINP2_47_QIO1);     // QSPI MISO
                P2[43].function(PINP2_43_QIO0);     // QSPI MOSI

                // The QuadSPI SPI mode functionality can be tested with a simple jumper
                // from P2[47] to P2[43] on the MOD-DEV70/100

                // Create and initialize semaphore for SPI (optional)
                OS_SEM QUADSPI_SEM;

                /* Initialize QuadSPI peripheral for Single-bit SPI with 2MHz bus speed and default SPI configuration:
                 *
                 * SPI_QSPI( uint8_t QSPIModule, uint32_t baudRateInBps,
                 *         uint8_t transferSizeInBits = 8, uint8_t peripheralChipSelects = 0x00,
                 *         uint8_t chipSelectPolarity = 0x0F, uint8_t clockPolarity = 0,
                 *         uint8_t clockPhase = 1, BOOL doutHiz = TRUE,
                 *         uint8_t csToClockDelay = 0, uint8_t delayAfterTransfer = 0 );
                 */
                SPI_QSPI mySPIObject(DEFAULT_QUADSPI_MODULE, 2000000);

                // Register the semaphore with the SPI driver to be able to pend on transaction completion instead of polling.
                mySPIObject.RegisterSem(&QUADSPI_SEM);

                // Open UART1 and get FD
                int uartFd = OpenSerial( 1, 115200, 1, 8, eParityNone );
        while (1)
        {
        	//getchar();
        	//PrintDhcpClients( MyServer );

        	if(dataavail(uartFd))
        	        {
        	            int num = read(uartFd, (char*)TXBuffer, 10000); // Read data from UART1

        	            mySPIObject.Start(TXBuffer, RXBufferspi, num);     // Write data read from UART1 to the QuadSPI module

        	            QUADSPI_SEM.Pend( 0 );                          // Wait for SPI transaction to complete

        	            writeall(uartFd, (char*)RXBufferspi, num);         // Send SPI RX via UART1
        	        }

        	OSTimeDly(TICKS_PER_SECOND);
        }
}
