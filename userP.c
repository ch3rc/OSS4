//======================================================
//Date:		March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		userP.c
//======================================================

#include "memory.h"

#define PERM (IPC_CREAT | 0666)

//======================================================
//Shared Memory
//======================================================

key_t clockKey = CLOCK;
key_t pcbKey = PCBKEY;
key_t pTableKey = PTABLE;
key_t semKey = SEM;

size_t clockSize = sizeof(Clock);
size_t pcbSize = sizeof(PCB);
size_t pTableSize = sizeof(Ptable);
size_t semSize = sizeof(sem_t);

int clockID = 0;
int pcbID = 0;
int pTableID = 0;
int semID = 0;

Clock *clockPtr = NULL;
PCB *pcbPtr = NULL;
Ptable *pTablePtr = NULL;
sem_t *semPtr = NULL;

void *getSharedMem(key_t, size_t, int *);
sem_t *getSharedSem(key_t, size_t, int *);
void detachAll();
void removeMem(void *);

//======================================================

int main(int argc, char *argv[])
{
	fprintf(stderr, "getting shared memory in child\n\n");

	clockPtr = (Clock *)getSharedMem(clockKey, clockSize, &clockID);
	pcbPtr = (PCB *)getSharedMem(pcbKey, pcbSize, &pcbID);
	pTablePtr = (Ptable *)getSharedMem(pTableKey, pTableSize, &pTableID);
	semPtr = getSharedSem(semKey, semSize, &semID);

	fprintf(stderr, "shared memory acquired successfully in child\n\n");

	sleep(1);

	fprintf(stderr, "detaching memory in child\n\n");

	detachAll();

	fprintf(stderr, "child finished!!!\n\n");

	return 0;
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

			case PCBKEY:
			perror("ERROR: userP: shmget(PCB)\n");
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

			case PCBKEY:
			perror("ERROR: userP: shmat(PCB)\n");
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

sem_t *getSharedSem(key_t key, size_t size, int *shmid)
{

	*shmid = shmget(key, size, PERM);

	if(*shmid < 0)
	{
		perror("ERROR: userP: shmget(sem_t)\n");
		exit(1);
	}

	sem_t *temp = (sem_t *)shmat(*shmid, 0, 0);
	if(temp == (sem_t *)-1)
	{
		perror("ERROR: userP: shmat(sem_t)\n");
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
		removeMem(clockPtr);
	}

	if(pcbID > 0)
	{
		removeMem(pcbPtr);
	}

	if(pTableID > 0)
	{
		removeMem(pTablePtr);
	}

	if(semID > 0)
	{
		removeMem(semPtr);
	}
}
