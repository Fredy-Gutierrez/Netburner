/*
 * Fifo.cpp
 *
 *  Created on: 9 abr 2021
 *      Author: Integra Fredy
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <nbrtos.h>
#include "fifo/Fifo.h"
#include "headers/Default_Values.h"

Fifo::Fifo() {
	// TODO Auto-generated constructor stub
	//Initiate();
	Initiate();
}

Fifo::~Fifo() {
	// TODO Auto-generated destructor stub
}

//starts or deletes the Fifo's list  pointers
void Fifo::Initiate(){
	while(Get()){
		MoveReader(true);
	}
	writer = -1;
	reader = -1;
	overflow = FALSE;
}

//Get the size of the Fifo list
int Fifo::FifoSize(){
	int size;
	if(overflow){
		size = FIFOBUFFERSIZE;
	}
	else{
		if(writer == reader){
			size = 0;
		}
		else{
			size = writer - reader;
		}
		if(size < 0){
			size = FIFOBUFFERSIZE + size;
		}
	}
	return size;
}


void Fifo::Add(void *pd){
	writer++;
	if(writer >= FIFOBUFFERSIZE)	{
		writer = 0;
		if(reader == -1){
		  reader = writer + 1;
		  overflow = TRUE;
		}
	}
	fifo[writer].pointer = pd;
	fifo[writer].rep = 0;
	/*check if the oldest change should be deleted,
	 * occurs when the reader is equal to the writer, that is, nothing was read or reported*/
	if(writer == reader){
		reader = writer + 1;
		overflow = TRUE;
		if(reader >= FIFOBUFFERSIZE)		{
			reader = 0;
		}
	}
	if(overflow){
	  //do something
	}
}

/*
void Fifo::Add(void *pd, int _groupCode){
	writer++;
	if(writer >= FIFOBUFFERSIZE)	{
		writer = 0;
		if(reader == -1){
		  reader = writer + 1;
		  overflow = 1;
		}
	}

	fifo[writer].pointer = pd;
	fifo[writer].rep = 0;
	fifo[writer].groupCode= _groupCode;

	//check if the oldest change should be deleted,
	// occurs when the reader is equal to the writer, that is, nothing was read or reported
	if(writer == reader){
		reader = writer + 1;
		overflow = 1;
		if(reader >= FIFOBUFFERSIZE)		{
			reader = 0;
		}
	}

	if(overflow){
	  //do something
	}
}
*/


void * Fifo::Get(){

	void * pd = NULL;

	int size, readerAux;

	size = FifoSize();

	if(size == 0){
		return pd;
	}

	readerAux = reader;
	readerAux++;

	if(readerAux >= FIFOBUFFERSIZE){
		readerAux = 0;
	}

	pd = fifo[readerAux].pointer;

	fifo[readerAux].rep = 1;

	return pd;
}

/*
int Fifo::GetTypeOfData(){
	int size, readerAux;

	size = FifoSize();

	if(size == 0){
		return -1;
	}

	readerAux = reader;
	readerAux++;

	if(readerAux >= FIFOBUFFERSIZE){
		readerAux = 0;
	}

	return fifo[readerAux].groupCode;

}

*/
/*This function increments the reader
 * if the process where the last value was used was successful,
 * it must be called to read the next value*/
void Fifo::MoveReader(bool freeMemory) {
	int readerAux, size;
	while(reader != writer)	{
		readerAux = reader;
		//points to the value to remove
		reader = reader + 1;

		//check if the pointer (reader) is bigger than or equal to the maximum
		if(reader >= FIFOBUFFERSIZE)	{
			reader = 0;
		}
		if(!fifo[reader].rep){
			reader = readerAux;
			break;
		}
		if(freeMemory)
			free(fifo[reader].pointer);//after the free, pointer becomes a dangling pointer
		fifo[reader].pointer = NULL;
	}
	//check if the overflow should be deleted
	if(overflow){
		if(reader == writer){
			overflow = FALSE;
		}
		else{
			size = writer - reader;
			if(size < 0){
				size = FIFOBUFFERSIZE + size + 1;
			}
			if(size < FIFOBUFFERSIZE){
				overflow = FALSE;
			}
		}
	}
}







