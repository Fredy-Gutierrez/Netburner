/*
 * Sargas_Connection.h
 *
 *  Created on: 25 feb 2021
 *      Author: Integra Fredy
 */

#ifndef LIBS_SARGAS_CONNECTION_H_
#define LIBS_SARGAS_CONNECTION_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//<body>

	void initSargasConnection();
	void SargasRfOn();
	void SargasRfOff();
	void SargasSetTimeAndDate();
	void closeFd();

//</body>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIBS_SARGAS_CONNECTION_H_ */
