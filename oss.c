//======================================================
//Date:		March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		oss.c
//======================================================


#include "memory.h"

#define PERM (IPC_CREAT | 0666)

//=====================================================
//constants
//=====================================================

pid_t childPid;
int exitPid;
int status = 0;

//====================================================
//shared memory
//====================================================

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

void *makeSharedMem(key_t, size_t, int *);
sem_t *makeSharedSem(key_t, size_t, int *);
void cleanMem(void *, int);
void clearAll();
//=====================================================

int main(int argc, char *argv[])
{
	fprintf(stderr, "getting shared memory in oss\n\n");
	
	clockPtr = (Clock *)makeSharedMem(clockKey, clockSize, &clockID);
	pcbPtr = (PCB *)makeSharedMem(pcbKey, pcbSize, &pcbID);
	pTablePtr = (Ptable *)makeSharedMem(pTableKey, pTableSize, &pTableID);
	semPtr = makeSharedSem(semKey, semSize, &semID);

	fprintf(stderr, "shared memory acquired in oss\n\n");

	childPid = fork();

	if(childPid < 0)
	{
		perror("ERROR: oss: fork\n");
		exit(1);
	}

	if(childPid == 0)
	{
		char str[20];
		snprintf(str, sizeof(str), "%d", 1);
		execl("./userP", str,  NULL);
	}

	if((exitPid = waitpid((pid_t)-1, &status, 0)) > 0)
	{
		if(WIFEXITED(status))
		{
			if(WEXITSTATUS(status) == 12)
			{
				fprintf(stderr,"child exited\n\n");
			}
		}
	}

	clearAll();

	fprintf(stderr, "oss finished\n\n\n");	

	return 0;
}

void *makeSharedMem(key_t key, size_t size, int *shmid)
{

	*shmid = shmget(key, size, PERM);
	
	if(*shmid < 0)
	{
		switch(key)
		{
			case CLOCK:
			perror("ERROR: oss: shmget(Clock)\n");
			break;

			case PCBKEY:
			perror("ERROR: oss: shmget(PCB)\n");
			break;

			case PTABLE:
			perror("ERROR: oss: shmget(Ptable)\n");
		}
		clearAll();
		exit(1);
	}

	void *temp = shmat(*shmid, NULL, 0);
	
	if(temp == (void *)-1)
	{
		switch(key)
		{
			case CLOCK:
			perror("ERROR: oss: shmat(Clock)\n");
			break;

			case PCBKEY:
			perror("ERROR: oss shmat(PCB)\n");
			break;
		
			case PTABLE:
			perror("ERROR: oss shmat(Ptable)\n");
			break;
		}
		clearAll();
		exit(1);
	}

	return temp;
}

sem_t *makeSharedSem(key_t key, size_t size, int *shmid)
{
	*shmid = shmget(key, size, PERM);
	if(*shmid < 0)
	{
		perror("ERROR: oss: shmget(sem_t)\n");
		exit(1);
	}

	sem_t *temp = (sem_t *)shmat(*shmid, NULL, 0);
	if(temp == (sem_t *)-1)
	{
		perror("ERROR: oss: shmat(sem_t)\n");
		exit(1);
	}

	if(sem_init(temp, 1, 1) == -1)
	{
		perror("ERROR: oss: sem_init\n");
		exit(1);
	}

	return temp;
}

void cleanMem(void *ptr, int shmid)
{
	if(shmdt(ptr) == -1)
	{
		perror("ERROR: oss: shmdt\n");
		exit(1);
	}

	if(shmctl(shmid, IPC_RMID, NULL) == -1)
	{
		perror("ERROR: oss: shmctl\n");
		exit(1);
	}
}

void clearAll()
{
	if(clockID > 0)
	{
		cleanMem(clockPtr, clockID);
	}
	
	if(pcbID > 0)
	{
		cleanMem(pcbPtr, pcbID);
	}
	
	if(pTableID > 0)
	{
		cleanMem(pTablePtr, pTableID);
	}

	if(semID > 0)
	{
		cleanMem(semPtr, semID);
	}
}
