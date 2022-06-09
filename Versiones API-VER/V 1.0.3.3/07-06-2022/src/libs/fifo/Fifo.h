/*
 * Fifo.h
 *
 *  Created on: 9 abr 2021
 *      Author: Integra Fredy
 */

#ifndef FIFO_H_
#define FIFO_H_

#define MAX_BUFFER_INFO 2000
#define MAX_BUFFER_CMD 2000

#include "libs/Default_Values.h"

class Fifo{

private:

	struct fifoStruct{
		void * pointer;
		int groupCode;
		int rep;
	};

	struct fifoStruct fifo[FIFOBUFFERSIZE];

	bool overflow;
	int writer;//index which take control of write process
	int reader;//index which take control of read process

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

typedef struct{
	typeAeiTime chgTime;
	int format;
	uint32_t par1;
	uint32_t par2;
}info;





class FifoInfo{

private:
	info list[MAX_BUFFER_INFO];

	bool overflow;
	int writer;//index which take control of write process
	int reader;//index which take control of read process

public:

	FifoInfo();
	virtual ~FifoInfo();

	void Initiate();
	int Size();
	void Add(uint32_t format, void * obj);
	bool Get(info * inf);

};

class FifoCmd{

private:
	info list[MAX_BUFFER_CMD];

	bool overflow;
	int writer;//index which take control of write process
	int reader;//index which take control of read process

public:

	FifoCmd();
	virtual ~FifoCmd();

	void Initiate();
	int Size();
	void Add(uint32_t format, void * obj);
	bool Get(info * command);

};

#endif /* FIFO_H_ */
