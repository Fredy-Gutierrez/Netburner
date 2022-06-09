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
#include "libs/log.h"
#include "libs/Rtc_Controller.h"
#include "libs/SargasProcess.h"
#include "libs/Business.h"
#include "libs/IO_Interface.h"
#include "libs/fifo/Fifo.h"
#include "libs/Default_Values.h"

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


/************************************* FIFO FOR INFORMATION ***********************************************/

FifoInfo::FifoInfo() {
	Initiate();
}

FifoInfo::~FifoInfo() {
	// TODO Auto-generated destructor stub
}

void FifoInfo::Initiate(){
	writer = -1;
	reader = -1;
	overflow = FALSE;
}

int FifoInfo::Size(){
	int size;
 	if(overflow) {
		size = MAX_BUFFER_INFO;
	}
	else {
		if(writer == reader){
			size = 0;
		}
		else {
			size = writer - reader;
		}
		if(size < 0){
			size = MAX_BUFFER_INFO + size;
		}
	}
	return size;
}

//add a value to the list
void FifoInfo::Add(uint32_t format, void * obj){
	digInput * dig;
	info * axle;

	writer++;
	if(writer >= MAX_BUFFER_INFO)	{
		writer = 0;
		if(reader == -1){
		  reader = writer + 1;
		  overflow = TRUE;
		}
	}
	switch(format){
		case repDigChg:
			dig = (digInput *)obj;
			list[writer].format = format;
			list[writer].par1 = dig->bit;
			list[writer].par2 = dig->state;
			list[writer].chgTime.sysTime = dig->chgTime.sysTime;
			list[writer].chgTime.msCurrSec = dig->chgTime.msCurrSec;
		break;
		case turnOnAnt:
		case turnOffAnt:
			list[writer].format = format;
			list[writer].chgTime.sysTime = time(0);
			list[writer].chgTime.msCurrSec = getMsTime();
		break;
		case repMainAxle:
		case repSecAxle:
		case repMirrorAxle:
			axle = (info *)obj;
			list[writer].format = format;
			list[writer].par1 = axle->par1;
			list[writer].par2 = axle->par2;
			list[writer].chgTime.sysTime = axle->chgTime.sysTime;
			list[writer].chgTime.msCurrSec = axle->chgTime.msCurrSec;
		break;
	}

	if(writer == reader) {
		reader = writer + 1;
		overflow = TRUE;
		if(reader >= MAX_BUFFER_INFO)	{
			reader = 0;
		}
	}

	if(overflow) {
	  //do something
	}

}

//Read the next val in the fifo list (turn on the reported flag)

bool  FifoInfo::Get(info * inf){

	if(Size() == 0){
		return FALSE;
	}
	reader++;

	if(reader >= MAX_BUFFER_INFO){
		reader = 0;
	}
	inf->format=list[reader].format;
	inf->par1=list[reader].par1;
	inf->par2=list[reader].par2;
	inf->chgTime.sysTime=list[reader].chgTime.sysTime;
	inf->chgTime.msCurrSec=list[reader].chgTime.msCurrSec;
	return TRUE;
}


/************************************* FIFO FOR COMMANDS ***********************************************/

FifoCmd::FifoCmd() {
	Initiate();
}

FifoCmd::~FifoCmd() {
	// TODO Auto-generated destructor stub
}

//starts or deletes the Fifo's list  pointers
void FifoCmd::Initiate(){
	writer = -1;
	reader = -1;
	overflow = FALSE;
}

int FifoCmd::Size(){
	int size;
	if(overflow) {
		size = MAX_BUFFER_CMD;
	}
	else {
		if(writer == reader){
			size = 0;
		}
		else {
			size = writer - reader;
		}
		if(size < 0){
			size = MAX_BUFFER_CMD + size;
		}
	}
	return size;
}

void FifoCmd::Add(uint32_t format, void * obj){
	info * infoRx;

	writer++;
	if(writer >= MAX_BUFFER_CMD)	{
		writer = 0;
		if(reader == -1){
		  reader = writer + 1;
		  overflow = TRUE;
		}
	}
	switch(format){
		case turnOnAnt:
			list[writer].format = turnOnAnt;
		break;
		case turnOffAnt:
			list[writer].format = turnOffAnt;
		break;
		case repMainAxle:
		case repSecAxle:
		case repMirrorAxle:
			infoRx = (info *)obj;
			list[writer].format = format;
			list[writer].par1 = infoRx->par1;
			list[writer].par2 = infoRx->par2;
			list[writer].chgTime.sysTime = infoRx->chgTime.sysTime;
			list[writer].chgTime.msCurrSec = infoRx->chgTime.msCurrSec;
		break;
	}

	if(writer == reader) {
		reader = writer + 1;
		overflow = TRUE;
		if(reader >= MAX_BUFFER_CMD)	{
			reader = 0;
		}
	}
	if(overflow) {
	  //do something
	}

}

bool  FifoCmd::Get(info * command){

	if(Size() == 0){
		return FALSE;
	}
	reader++;

	if(reader >= MAX_BUFFER_CMD){
		reader = 0;
	}
	command->format=list[reader].format;
	command->par1=list[reader].par1;
	command->par2=list[reader].par2;
	command->chgTime.sysTime=list[reader].chgTime.sysTime;
	command->chgTime.msCurrSec=list[reader].chgTime.msCurrSec;
	return TRUE;
}






