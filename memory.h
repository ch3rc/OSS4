//===================================================
//Date: 	March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		memory.h
//===================================================

#ifndef MEMORY_H
#define MEMORY_H

#define CLOCK 0x11111111
#define PTABLE 0x33333333
#define MEM1 0x44444444
#define MEM2 0x55555555

typedef struct{

	unsigned int n;
	unsigned int s;
}Clock;

typedef struct{

	int pid;
	int sid;
	int quant;
	int p;
	int ct;
	int stat;
	Clock tcpu;
	Clock tsys;
	Clock burst;
	Clock bt;
	Clock wt;
}PCB;

typedef struct{

	int quant;
	PCB stack[18];

}Ptable;

typedef struct{

	long mtype;
	char msg[100];

}Message;

struct Queue{

	int pid;
	struct Queue *next;
	struct Queue *prev;
};



void launchClock(Clock *,int, int);
void moveClock(Clock *, int, int);
void avgTime(Clock *, int);
void reset(int);
void insert(struct Queue**, int);
void deleteQ(struct Queue**, int);
void display(struct Queue *);

	
#endif
