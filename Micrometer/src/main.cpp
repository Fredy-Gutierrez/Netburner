#include <predef.h>
#include <stdio.h>
#include <nbrtos.h>
#include <http.h>
#include <init.h>
#include <basictypes.h>
#include "headers/Business.h"
#include "headers/Configurations.h"
#include "FatLib/FileSystemUtils.h"

const char * AppName="Micrometer";

void UserMain(void * pd)
{
	// Note that you almost never want to put anything before you call init()
	// but in this case, we need to call f_enterFS() before the HTTP task starts, or we won't
	// be able to do it.
	addFSTask(HTTP_PRIO, MAIN_PRIO);

    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait up to 5 seconds for active network activity 
    StartHttp();

    addFSTask(FTP_PRIO, MAIN_PRIO);//add the task for ftp process

    iprintf("Aplicacion %s iniciada\n",AppName );
    getConfigurations();
    initBusiness();
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
