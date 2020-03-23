//===================================================
//Date: 	March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		memory.h
//===================================================

#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <semaphore.h>

#define CLOCK 0x11111111
#define PCBKEY 0x22222222
#define PTABLE 0x33333333
#define SEM 0x44444444

typedef struct{

	unsigned int nano;
	unsigned int seconds;
}Clock;

typedef struct{

	int processID;
	int simulatedPID;
	int priority;
	Clock totalTimeCPU;
	Clock totalTimeInSys;
	Clock lastBurst;
}PCB;

typedef struct{
	
	PCB stack[18];
	unsigned int quantum;
	int state;

}Ptable;

	
#endif
