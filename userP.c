//======================================================
//Date:		March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		userP.c
//======================================================

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include "memory.h"

#define PERM (IPC_CREAT | 0666)

const int percentage = 20;

//======================================================
//Shared Memory
//======================================================

key_t clockKey = CLOCK;
key_t pTableKey = PTABLE;

size_t clockSize = sizeof(Clock);
size_t pTableSize = sizeof(Ptable);


int clockID = 0;
int pTableID = 0;

int msgID = 0;
int osSMsg;
int usrRcv;

Clock *clockp = NULL;
Ptable *table = NULL;
Message msg;

void *getSharedMem(key_t, size_t, int *);
void detachAll();
void removeMem(void *);
void launch();
void message();
int throwMsg(int, Message *, int);
//======================================================

int main(int argc, char *argv[])
{
	srand(time(0) ^ (getpid() << 16));

	int p = atoi(argv[0]);
	int random;
	int term = 15;
		
	message();

	launch();
	int aS = clockp->s;
	int aN = clockp->n;
	
	fprintf(stderr, "\nNOW IN CHILD PROCESS at position = %d\n", p);

	if(msgrcv(usrRcv, &msg, sizeof(msg), table->stack[p].pid, IPC_NOWAIT)> 0)
	{	
		while(strcmp(msg.msg, "RUN") != 0)
			;
	
		if((rand() % 100) <= term)
		{
			
			//determine the slice of
			random  = (rand() % table->quant);
			table->stack[p].quant = random;
			table->stack[p].tcpu.n = clockp->n - (aN + random);
			table->stack[p].tcpu.s = clockp->s - aS;
			table->stack[p].tsys.s = clockp->s - table->stack[p].burst.s;
			table->stack[p].tsys.n = clockp->n - table->stack[p].burst.n;
			table->stack[p].wt.s = clockp->s - table->stack[p].burst.s;
			table->stack[p].wt.n = clockp->n - (table->stack[p].burst.n + random);
			msg.mtype = table->stack[p].pid;
			strcpy(msg.msg, "TERMINATE");
			msgsnd(osSMsg, &msg, sizeof(msg), IPC_NOWAIT);
			fprintf(stderr, "\nPID %d Terminating and using part of quantum\n",
			table->stack[p].sid);

			msg.mtype = table->stack[p].pid;
			char str[20];
			snprintf(str, sizeof(str), "%d", random);
			strcpy(msg.msg, str);
			msgsnd(osSMsg, &msg, sizeof(msg), IPC_NOWAIT);
			detachAll();
			exit(12);
		}

		else
		{
			msg.mtype = table->stack[p].pid;
			strcpy(msg.msg, "EXHAUSTED");
			msgsnd(osSMsg, &msg, sizeof(msg), 0);
			fprintf(stderr, "\nPID %d using entire quantum\n", table->stack[p].sid);	
		}
	
	}		
		

}

void launch()
{
	clockp = (Clock *)getSharedMem(clockKey, clockSize, &clockID);
	table = (Ptable *)getSharedMem(pTableKey, pTableSize, &pTableID);
}

int throwMsg(int id, Message *buf, int size)
{
	int temp;
	if((temp = msgsnd(id, buf, size, IPC_NOWAIT)) == -1)
	{
		return -1;
	}
	return temp;
}

void message()
{
	key_t ossMsg = MEM1;
	key_t usrMsg = MEM2;
	
	osSMsg = msgget(ossMsg, PERM);
	if(osSMsg == -1)
	{
		perror("ERROR: userP: msgget\n");
		exit(1);
	}

	usrRcv = msgget(usrMsg, PERM);
	if(usrRcv == -1)
	{
		perror("ERROR: userP: msgget\n");
		exit(1);
	}
}

void *getSharedMem(key_t key, size_t size, int *shmid)
{
	*shmid = shmget(key, size, PERM);

	if(*shmid < 0)
	{
		switch(key)
		{
			case CLOCK:
			perror("ERROR: userP: shmget(Clock)\n");
			break;

			case PTABLE:
			perror("ERROR: userP: shmget(Ptable)\n");
			break;

			
		}
		detachAll();
		exit(1);
	}

	void *temp = shmat(*shmid, NULL, 0);

	if(temp == (void *)-1)
	{
		switch(key)
		{
			case CLOCK:
			perror("ERROR: userP: shmat(Clock)\n");
			break;

			case PTABLE:
			perror("ERROR: userP: shmat(Ptable)\n");
			break;

			
		}
		detachAll();
		exit(1);
	}

	return temp;
}

void removeMem(void *ptr)
{
	if(shmdt(ptr) == -1)
	{
		perror("ERROR: userP: shmdt\n");
		exit(1);
	}
}

void detachAll()
{

	if(clockID > 0)
	{
		removeMem(clockp);
	}

	if(pTableID > 0)
	{
		removeMem(table);
	}
}
