/*
 * Default_Values.h
 *
 *  Created on: 4 mar 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_DEFAULT_VALUES_H_
#define LIBS_DEFAULT_VALUES_H_

#include <constants.h>

/********************************APP VALS*******************************/
#define APPVERSION "1.0.3.3Beta"

/********************************APP TASK PRIORITIES*******************************/
/*		Factory applications use priorities 46 through 56 which main is 50.
 *
 *      priorities:
 *      Sargas tcp MAIN +1 					= 51
 *      Report2Server MAIN + 2				= 52
 *      Ntptask(Rtc_Controller) MAIN +3		= 53
 *      CommandTask MAIN +4					= 54
 *      TagProcessorTask MAIN + 5			= 55 MAX
 *
 *      if you want add more task, you must use MAIN_PRIO - n, n must not be > 3
*/
#define SARGAS_PRIO MAIN_PRIO-3
#define REPORT_SERVER_PRIO MAIN_PRIO-2
#define INT_SERVICES_PRIO MAIN_PRIO-1
// MAIN_PRIO  50
#define TAG_PROCESS_PRIO MAIN_PRIO+1
#define DBTRAIN_PRIO MAIN_PRIO+2
#define NTP_TASK_PRIO MAIN_PRIO+3
#define FREE2_PRIO MAIN_PRIO+4
#define TERMINAL_PRIO MAIN_PRIO+5

/********************************APP STATIC IP*******************************/
#define DEFAULT_NB_IP_MODE "DHCP"
#define DEFAULT_NB_IP_ADD "192.168.15.200"
#define DEFAULT_NB_IP_MASK "255.255.255.0"
#define DEFAULT_NB_IP_GATEWAY "192.168.15.1"
#define DEFAULT_NB_IP_DNS1 "192.168.15.1"

/********************************SARGAS PORTS AND IP*******************************/
#define DEFAULT_SARGAS_IP "192.168.15.99"
#define DEFAULT_SARGAS_ADMIN_PORT 2711
#define DEFAULT_SARGAS_USER_PORT 2712

#define DEFAULT_LOG_OUT 1
#define DEFAULT_UDP_IP "192.168.15.152"
#define DEFAULT_UDP_PORT 9999

#define DEFAULT_AEI_DIR true
#define DEFAULT_AEI_ID "INTEGRA"

#define DEFAULT_SERVER_URL "https://operacion.ferrovalle.com.mx/AeiSvrWs/operation/AeiWs.aspx"

#define DEFAULT_NTP_SYNC_ONOFF true
#define DEFAULT_NTP_SERVER_NAME "pool.ntp.org"
#define DEFAULT_NTP_SYNC_TIME 1

#define DEFAULT_LOOP_TYPE 0
#define DEFAULT_LOOP_TIME_ON 300
#define DEFAULT_LOOP_TIME_OFF 120
#define DEFAULT_LOOP_RFID_ONOFF true

#define DEFAULT_AXIES_DET_SIGNAL 0
#define DEFAULT_PHONE_NUMBER "UNKNOWN"

#define DEFAULT_BATT_INSTALL_DATE 1621036800
#define DEFAULT_CAR_COUNTER true

#define DEFAULT_BATT_TEST_ONOFF true
#define DEFAULT_BATT_TEST_PERIOD 3
#define DEFAULT_BATT_TEST_HOUR 3
#define DEFAULT_BATT_TEST_LAST 0

#define DEFAULT_TEMP_ALARM_SET 30
#define DEFAULT_TEMPERATURE_ALARM_ONOFF true

#define DEFAULT_IN_SYS1 3
#define DEFAULT_IN_SYS1P 2
#define DEFAULT_IN_SYS1_N 1
#define DEFAULT_IN_SYS1_NP 6
#define DEFAULT_IN_SYS2 17
#define DEFAULT_IN_SYS2P 7
#define DEFAULT_IN_SYS2_N 5
#define DEFAULT_IN_SYS2_NP 10
#define DEFAULT_IN_DCOK1 13
#define DEFAULT_IN_DCOK2 12
#define DEFAULT_IN_ACOK1 11
#define DEFAULT_IN_BAT1 16
#define DEFAULT_IN_ACOK2 15
#define DEFAULT_IN_BAT2 14
#define DEFAULT_IN_DOOR 4
#define DEFAULT_IN_FREE 18

#define DEFAULT_DOUBLE_AX_DET false

#define FIFOBUFFERSIZE 1000

#define DEFAULT_QTY_SARGAS 1
#define DEFAULT_SARGAS_IP_R "192.168.15.99"
#define DEFAULT_SARGAS_IP_L "192.168.15.100"
#define DEFAULT_QTY_SARGAS_ANT_R 2
#define DEFAULT_QTY_SARGAS_ANT_L 2

#define DEFAULT_MAIN_AXIES_DET_SIGNAL 0
#define DEFAULT_DOUBLE_AXIES_DET false
#define DEFAULT_DOUBLE_AXIES_DET_SIGNAL 0
#define DEFAULT_MIRROR_AXIES_DET false
#define DEFAULT_MIRROR_AXIES_DET_SIGNAL 2

#define DEFAULT_DIR_DOUBLE_AD 1
#define DEFAULT_DIR_MIRROR_AD 1

#define DEFAULT_IN_LOOPC_I 9
#define DEFAULT_IN_LOOPO 8

#endif /* LIBS_DEFAULT_VALUES_H_ */
