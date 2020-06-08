#include <predef.h>
#include <stdio.h>
#include <nbrtos.h>
#include <http.h>
#include <init.h>


const char * AppName="Proff";

void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait up to 5 seconds for active network activity 
    StartHttp();

    iprintf("Application %s started\n",AppName );
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
