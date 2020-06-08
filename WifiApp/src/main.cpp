#include <predef.h>
#include <stdio.h>
#include <nbrtos.h>
#include <http.h>
#include <init.h>
#include <wifi/wifi.h>


const char * AppName="WifiApp";

// Enable the NBWIFIIN-SPI driver
wifi_init wifiInit = {
    NBWIFI_PLAT_DEFAULT_SPINUM,       NBWIFI_PLAT_DEFAULT_CONNUM,
    NBWIFI_PLAT_DEFAULT_CSNUM,        NBWIFI_PLAT_DEFAULT_PINNUM,
    NBWIFI_PLAT_DEFAULT_RESETPIN,     NBWIFI_PLAT_DEFAULT_IRQNUM,
    NBWIFI_PLAT_DEFAULT_UART,         NBWIFI_PLAT_DEFAULT_CHIPEN,
    NB::NBWifi::GetNewNBWifiSPIDriver};

void UserMain(void * pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait up to 5 seconds for active network activity 
    StartHttp();
    InitWifi_SPI("<SSID>","<Password>"); // TODO: Fill in your local SSID and password to initialize and connect

    iprintf("Application %s started\n",AppName );
    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
