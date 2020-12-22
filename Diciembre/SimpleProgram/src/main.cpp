/* Revision: 3.2.0 */

/******************************************************************************
* Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/

#include <system.h>
#include <predef.h>
#include <stdio.h>
#include <tcp.h>
#include <nbrtos.h>
#include <iosys.h>
#include <fdprintf.h>
#include <init.h>
#include <serial.h>
#include <startnet.h>
#include <constants.h>


extern "C"
{

}

const char *AppName = "Simple HTML Example";

OS_SEM MySemaphore;   // Create semaphore

/**
 *  UserMain
 *
 *  Main entry point for the example
 */

#define TCP_LISTEN_PORT 4444   // Telent port number
#define RX_BUFSIZE (4096)

char RXBuffer[RX_BUFSIZE];

char * msg="";

char * msgserial="";


// Allocate task stack for TCP listen task
uint32_t   TcpServerTaskStack[USER_TASK_STK_SIZE];

/*-------------------------------------------------------------------
 * TCP Server Task
 *------------------------------------------------------------------*/
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

                    msg = RXBuffer;
                } while ( n > 0 );

                iprintf("Closing client connection: %I\r\n", GetSocketRemoteAddr(fdAccept) );
                close(fdAccept);
                fdAccept = 0;
            }
            MySemaphore.Post();
        } // while(1)
    } // while listen
}

void WebDestPort(int sock, PCSTR url)
{
    fdprintf(sock, "%s ", msg);
}

void WebSerialPort(int sock, PCSTR url)
{
    fdprintf(sock, "%s", msgserial);
}

void UserMain(void *pd)
{
    init();                                       // Initialize network stack

    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork();

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    MySemaphore.Init();

    OSTaskCreatewName( TcpServerTask,
                           (void  *)TCP_LISTEN_PORT,
                           &TcpServerTaskStack[USER_TASK_STK_SIZE] ,
                           TcpServerTaskStack,
                           MAIN_PRIO - 1,   // higher priority than UserMain
                           "TCP Server" );
    SerialClose(0);
    SerialClose(1);

    // Open the serial ports....
    int fd0 = OpenSerial(0, 115200, 1, 8, eParityNone);
    int fd1 = OpenSerial(1, 115200, 1, 8, eParityNone);

    // Now write something out both ports
    writestring(fd1, "Test1");
    writestring(fd0, "Test0");

    while (1)
    {
    	MySemaphore.Pend(TICKS_PER_SECOND * 15); //funcion para detener el semaforo hasta el tiempo marcado o hasta que el post lo libere
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
    }
}

