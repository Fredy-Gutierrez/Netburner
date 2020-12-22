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

#include <http.h>
#include <iosys.h>
#include <httppost.h>
#include <fdprintf.h>
#include <string.h>

int FileSize;
const int FILE_BUFFER_SIZE=10000;
char FileRxBuffer[FILE_BUFFER_SIZE];



/**
 *  ShowTextStreamData
 *
 *  Display a stream of data on a web page as text with or without binary
 */
void ShowFileData(int sock, bool showBinary)
{
    uint8_t rbuf[16];
    int nread=0;
    int rv = 0;

    if (showBinary) { fdiprintf(sock, "<B><PRE>"); }

    do
    {
        //Copy 16 bytes at a time for binary type display
        rv=0;
        while((rv<16) && (nread<FileSize)) rbuf[rv++]=FileRxBuffer[nread++];

        if (rv > 0)
        {
            int n = 0;

            // Print out the binary data
            if (showBinary)
            {
                for (n = 0; n < rv; n++)
                {
                    fdiprintf(sock, "%02x   ", rbuf[n]);
                }
                for (n = n; n < 16; n++) // @suppress("Assignment to itself")
                {
                    fdiprintf(sock, "--   ");
                }
                fdiprintf(sock, "\r\n");
            }

            // Print out standard character data
            for (n = 0; n < rv; n++)
            {
                if (showBinary) { fdiprintf(sock, " "); }
                if ((rbuf[n] >= 0x20) && (rbuf[n] <= 0x80))
                {
                    switch (rbuf[n])
                    {
                        case '"':
                        {
                            fdiprintf(sock, "&quot;");
                            break;
                        }
                        case '&':
                        {
                            fdiprintf(sock, "&amp;");
                            break;
                        }
                        case '<':
                        {
                            fdiprintf(sock, "&lt;");
                            break;
                        }
                        case '>':
                        {
                            fdiprintf(sock, "&gt;");
                            break;
                        }
                        default:
                        {
                            fdiprintf(sock, "%c", rbuf[n]);
                            break;
                        }
                    }
                }
                else if (!showBinary && (rbuf[n] == '\r'))
                {
                    fdiprintf(sock, "<BR>\r\n");
                }
                else if (showBinary)
                {
                    fdiprintf(sock, ".");
                }

                // If we are printing out binary data too, do some extra formatting
                if (showBinary) { fdiprintf(sock, "   "); }
            }
            if (showBinary) { fdiprintf(sock, "\r\n\r\n"); }
        }
    } while (rv > 0);

    if (showBinary) { fdiprintf(sock, "<B><PRE>"); }
}


void ProcessPostFile(const char * pValue)
{
FilePostStruct * pFps=(FilePostStruct*)pValue;
FileSize=0;

while(dataavail(pFps->fd))
{
 int rs=FILE_BUFFER_SIZE-FileSize;
 int rv=read(pFps->fd,FileRxBuffer+FileSize,rs);
 if(rv>0) FileSize+=rv;
 if(rv<=0)  break;
 if(FileSize>=FILE_BUFFER_SIZE) break;
}
close(pFps->fd);
}





void PostCallBack( int sock,PostEvents event,const char * pName, const char * pValue)
{
static bool bShowBinary=false;
switch(event)
{
case eStartingPost:
    bShowBinary=false; //Init the variables
    FileSize=0;
    iprintf("Post start\r\n");
break;

//Only one variable to monitor cbox and if not checked its not sent.
case eVariable:
if(strcmp(pName,"cbox")==0) bShowBinary=true;
break;


case eFile:
//Reead the first 10K or so of the file into a buffer
ProcessPostFile(pValue);
break;


case eEndOfPost:
{
 SendHTMLHeader(sock);
 writestring(sock, "<HTML><BODY>");
 if(FileSize!=0)
 {
 writestring(sock, "<h2>Start of File</h2> <BR>");
 ShowFileData(sock,bShowBinary);
 writestring(sock, "<BR>End of File\r\n");
 }
else
{
 writestring(sock, "Unable to extract file <BR>\r\n");
}
writestring(sock, "</BODY></HTML>");


}
break;
}//Switch

}

//Register a call back for the web page...
HtmlPostVariableListCallback filePostForm("filepost.html",PostCallBack);



