/*
 * Default_Values.h
 *
 *  Created on: 4 mar 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_DEFAULT_VALUES_H_
#define LIBS_DEFAULT_VALUES_H_

/********************************APP VALS*******************************/
#define APPVERSION "1.0.0.3"
#define NTP_SERVER_NAME "pool.ntp.org"


/********************************APP TASK PRIORITIES*******************************/
/*
 *      priorities:
 *      Sargas tcp MAIN +1
 *      Report2Server MAIN + 2
 *      Ntptask(Rtc_Controller) MAIN +3
 *      CommandTask MAIN +4
*/

/********************************APP STATIC IP*******************************/
#define DEFAULTIP "192.168.15.199"
#define DEFAULTMASK "255.255.255.0"
#define DEFAULTGATEWAY "192.168.15.1"
#define DEFAULTDNS1 "192.168.15.1"

/********************************APP DEBUG PORT*******************************/
#define DEBUGPORT 1

/********************************SARGAS PORTS AND IP*******************************/
#define DEFAULTSARGASIP "192.168.15.99"
#define ADMINPORT 2711
#define USERPORT 2712

#define DEFAULTSERVERURL "http://operacion.ferrovalle.mx/AeiSvrWs/operation/AeiWs.aspx"



#endif /* LIBS_DEFAULT_VALUES_H_ */
