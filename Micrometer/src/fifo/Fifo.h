/*
 * Fifo.h
 *
 *  Created on: 9 abr 2021
 *      Author: Integra Fredy
 */

#ifndef FIFO_H_
#define FIFO_H_

#include "headers/Default_Values.h"

class Fifo{

private:

	struct fifoStruct{
		void * pointer;
		int groupCode;
		int rep;
	};

	struct fifoStruct fifo[FIFOBUFFERSIZE];

	bool overflow;
	int writer = -1;//index which take control of write process
	int reader = -1;//index which take control of read process

public:

	Fifo();
	virtual ~Fifo();
	void Initiate();
	int FifoSize();
	void Add(void *pd);
	//Esta función lee TODOS los cambios de la lista, enciende la bandera de rep de cada cambio leído
	void * Get();
	int GetTypeOfData();
	void MoveReader(bool freeMemory);
};

#endif
