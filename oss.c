//======================================================
//Date:		March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		oss.c
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
#define MAX 18
//=====================================================
//globals
//=====================================================

pid_t childPid;
int exitPid;
int status = 0;
int launched = 0;
int max = 100; //low for testing purposes
int echild = 0;
int dead = 0;
//used for randoms
int maxNan = 1000;
//overhead time
int oTime = 0;
//process launch time
int newNan = 0;
int newSec = 0;
//time between bursts
int newLaunch = 0;

int written = 1;
int timeUsed = 0;

int seat = 0;
//=====================================================
//constants
//=====================================================

const int maxTimeBetweenNewProcsNs = 1000;
const int maxTimeBetweenNewProcsSecs = 1;
const int percentage = 15;
const int baseQuant = 10000;

//=====================================================
//shared memory
//=====================================================

key_t clockKey = CLOCK;
key_t pTableKey = PTABLE;


size_t clockSize = sizeof(Clock);
size_t pTableSize = sizeof(Ptable);


int clockID = 0;
int pTableID = 0;


int osSMsg = 0;
int usrRcv = 0;

Clock *clockp  = NULL;
Ptable *table = NULL;
Message msg;


void *makeSharedMem(key_t, size_t, int *);
void cleanMem(void *, int);
void clearAll();

struct Queue *queue0 = NULL;
struct Queue *queue1 = NULL;
struct Queue *queue2 = NULL;
struct Queue *queue3 = NULL;
//struct Queue *blocked = NULL;


//=====================================================
//Local Functions
//=====================================================

void initClock();
void initTable();
int findSpot();
void message();
void launch();
void clearMessage();
int throwMsg(int, Message *, int);
void killAll(int);
void timesUp(int);
//=====================================================
int main(int argc, char *argv[])
{

	alarm(3);
	signal(SIGINT, killAll);
	signal(SIGALRM, timesUp);
	
	unsigned int quant0 = baseQuant;
	unsigned int quant1 = quant0 / 2;
	unsigned int quant2 = quant1 / 2;
	unsigned int quant3 = quant2 / 2;
	

	int q0 = 0;
	int q1 = 0;
	
	
	Clock generate = {0,0};
	Clock launchc = {0,0};
	Clock cputime = {0,0};
	Clock systime = {0,0};
	Clock wttime = {0,0};

	launch();

	initClock();
	
	initTable();

	message();
	
	/*FILE *fp;
	fp = fopen("log.dat", "a");
	if(fp == -1)
	{
		perror("ERROR: OSS: fopen\n");
		exit(1);
	}*/

	int i;
	int j = 0;
	int k = 0;

	srand(time(0));

	while(dead < 100)
	{
		//overhead time 1.xx
		moveClock(clockp, 1, (rand() % maxNan));

		//inital generation time
		if(generate.s == 0 && generate.n == 0)
		{
			moveClock(&generate, (rand() % 3), 0);
		}
			
			
		if((launched < MAX) && max != 0 && (clockp->s >= generate.s) && (clockp->n >= generate.n))
		{
			int bS = (rand() % maxTimeBetweenNewProcsSecs);
			int bN = (rand() % maxTimeBetweenNewProcsNs);
			moveClock(&generate, bS, bN);
			launchc.s = generate.s;
			launchc.n = generate.n;
		
			seat = findSpot();
			//if seat returns -1 skip launch until seat opens
			if(seat > -1)
			{
	
				childPid = fork();
	
				if(childPid < 0)
				{
					perror("ERROR: OSS: fork\n");
					exit(1);
				}
	
				if(childPid == 0)
				{
					char pos[20];
					snprintf(pos, sizeof(pos), "%d", seat);
					execl("./userP", pos, NULL);
				}

				launched++;
				echild++;
				max--;

				fprintf(stderr, "\nseat = %d, pid = %d launched = %d echild = %d max = %d\n", 
				seat, getpid(), launched, echild, max);
				
				//table->stack[seat].p = (rand() % 2);
				table->stack[seat].pid = getpid();
				table->stack[seat].sid = seat + 1;
				table->stack[seat].ct = ((rand() % 100) <= percentage) ? 0:1;
				table->stack[seat].stat = -1;
				table->stack[seat].tcpu.n = 0;
				table->stack[seat].tcpu.s = 0;
				table->stack[seat].tsys.n = 0;
				table->stack[seat].tsys.s = 0;
				table->stack[seat].burst.n = generate.n;
				table->stack[seat].burst.s = generate.s;	
				table->stack[seat].bt.n = 0;
				table->stack[seat].bt.s = 0;
				table->stack[seat].wt.n = 0;
				table->stack[seat].wt.s = 0;

					
				if(table->stack[seat].ct == 0)
				{
					table->stack[seat].p = 0;
					insert(&queue0, table->stack[seat].sid);	
							
					if(written < 1000)
					{
						fprintf(stderr, "OSS: Generating process %d and putting it in queue 0"
						" at time %d:%d\n", table->stack[seat].sid, clockp->s,clockp->n);
						written++;
					}
					else if(written >= 1000)
					{
						fprintf(stderr, "Log has reached 1000 lines, starting new file\n");
						written = 1;
					}
				}

				if(table->stack[seat].ct == 1)
				{
					table->stack[seat].p == 1;
					insert(&queue1, table->stack[seat].sid);
					if(written < 1000)
					{
						fprintf(stderr, "OSS: Generating process %d and putting it in queue 1"
						" at time %d:%d\n", table->stack[seat].sid, clockp->s,clockp->n);
						written++;
					}
					else if(written >= 1000)
					{
						fprintf(stderr, "Log has reached 1000 lines, starting new file\n");
						written = 1;
					}
				}
			}
		}
		
		if(clockp->s >= launchc.s && clockp->n >= launchc.n)
		{

			if(table->stack[k].p == 0 && table->stack[k].stat == -1)
			{
				
				msg.mtype = table->stack[k].pid;
				strcpy(msg.msg, "RUN");
				msgsnd(usrRcv, &msg, sizeof(msg), IPC_NOWAIT);
				table->quant = quant0;
				table->stack[k].stat = 1;
				if(written < 1000)
				{
					fprintf(stderr, "\nOSS: Dispatching Pid %d at time %d:%d number = %d\n",
					table->stack[k].sid, clockp->s, clockp->n, j);
					written++;
					j++;
					
					int temp = (rand() % (1000 - 100)) + 100;
					fprintf(stderr,"\nOSS: total time this dispatch was %d nanos\n",temp);
					written++;
					moveClock(clockp, 0, temp);
				}
				
			}
	
			else if(table->stack[k].p == 1 && table->stack[k].stat == -1)
			{
				msg.mtype = table->stack[k].pid;
				strcpy(msg.msg, "RUN");
				msgsnd(usrRcv, &msg, sizeof(msg), IPC_NOWAIT);
				table->quant = quant1;
				table->stack[k].stat = 1;
				if(written < 1000)
				{
					fprintf(stderr, "\nOSS: Distpatching Pid %d at time %d:%d\n",
					table->stack[k].sid, clockp->s, clockp->n);
					written++;
					int temp = (rand() % (1000 - 100)) + 100;
					fprintf(stderr, "\nOSS: total time this dispatch was %d nanos\n", temp);
					written++;
					moveClock(clockp, 0, temp);
				}
			}

			
		}
		
		if(msgrcv(osSMsg, &msg, sizeof(msg), table->stack[k].pid, IPC_NOWAIT) > 0)
		{
			
			if(strstr(msg.msg, "TERMINATE"))
			{	
				msgrcv(osSMsg, &msg, sizeof(msg), table->stack[k].pid, IPC_NOWAIT);
				int gran = atoi(msg.msg);
					
				if(table->stack[k].p == 0)
				{		
					deleteQ(&queue0, table->stack[k].sid);
					q0--;	
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS:Q0: Receiving that process with PID %d ran for %d nanoseconds\n",
						table->stack[k].sid, gran);
						written++;
					}
					moveClock(clockp, 0, gran);
				}
				else if(table->stack[k].p == 1)
				{
					deleteQ(&queue1, table->stack[k].sid);
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS: Q1: Receiving that proces with pid %d ran for %d nanos\n",
						table->stack[k].sid, gran);
						written++;
					}
					moveClock(clockp, 0, gran);
				}
				else if(table->stack[k].p == 2)
				{
					deleteQ(&queue2, table->stack[k].sid);
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS: Q2: Receiving that process with pid %d ran for %d nanos\n",
						table->stack[k].sid, gran);
						written++;
					}
					moveClock(clockp, 0, gran);
				}

				else if(table->stack[k].p == 3)
				{
					deleteQ(&queue3, table->stack[k].sid);
					if(written < 1000)
					{
						fprintf(stderr, "\OSS: Q3: Receiving that process with pid %d ran for %d nanos\n",
						table->stack[k].sid, gran);
					}
					moveClock(clockp, 0, gran);
				}

				cputime.s += table->stack[k].tcpu.s;
				cputime.n += table->stack[k].tcpu.n;
				systime.s += table->stack[k].tsys.s;
				systime.n += table->stack[k].tsys.n;
				wttime.s += table->stack[k].wt.s;
				wttime.n += table->stack[k].wt.n;	
		
				
				while((exitPid = waitpid((pid_t)-1, &status, 0)) > 0)
				{
					dead++;
					fprintf(stderr, "\nOSS: PID has terminated dead = %d\n", dead);
					reset(k);
					launched--;	
				}
				
			}
		}

		if(msgrcv(osSMsg, &msg, sizeof(msg), table->stack[k].pid, IPC_NOWAIT) > 0)
		{		
			if(strstr(msg.msg, "EXHAUSTED"))
			{
				if(table->stack[k].p == 0)
				{
					deleteQ(&queue0, table->stack[k].sid);
					insert(&queue0, table->stack[k].sid);
					table->stack[k].stat = -1;
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS: Putting process with PID %d into queue 0\n",
						table->stack[k].sid);
						written++;
					}
					moveClock(clockp, 0, quant0);
				}
				else if(table->stack[k].p == 1)
				{
					deleteQ(&queue1, table->stack[k].sid);
					insert(&queue2, table->stack[k].sid);
					table->stack[k].stat = -1;
					table->stack[k].p = 2;
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS: Putting process with PID %d into queue 2\n",
						table->stack[k].sid);
						written++;
					}
					moveClock(clockp, 0, quant1);
				}

				else if(table->stack[k].p == 2)
				{
					deleteQ(&queue2, table->stack[k].sid);
					insert(&queue3, table->stack[k].sid);
					table->stack[k].stat = -1;
					table->stack[k].p = 3;
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS: Putting process with pid %d int queue 3\n",
						table->stack[k].sid);
						written++;
					}
					moveClock(clockp, 0, quant2);
				}

				else if(table->stack[k].p == 3)
				{
					deleteQ(&queue3, table->stack[k].sid);
					insert(&queue3, table->stack[k].sid);
					table->stack[k].stat = -1;
					table->stack[k].p = 3;
					if(written < 1000)
					{
						fprintf(stderr, "\nOSS: putting process with pid %d into queue 3 again\n",
						table->stack[k].sid);
						written++;
					}
					moveClock(clockp, 0, quant3);
				}	
			}
			
		}

		/*if(queue0 != NULL || queue1 != NULL)
		{
			if(queue0 != NULL)
			{
				table->quant = quant0;
				table->stack[k].stat = 1;
				msg.mtype = table->stack[k].pid;
				strcpy(msg.msg,"RUN");
				msgsnd(usrRcv, &msg, sizeof(msg), IPC_NOWAIT);
			}
			if(queue1 != NULL)
			{
				table->quant = quant1;
				table->stack[k].stat = 1;
				msg.mtype = table->stack[k].pid;
				strcpy(msg.msg, "RUN");
				msgsnd(usrRcv, &msg, sizeof(msg), IPC_NOWAIT);
			}
			if(queue2 != NULL)
			{
				table->quant = quant2;
				table->stack[k].stat = 1;
				msg.mtype = table->stack[k].pid;
				strcpy(msg.msg, "RUN");
				msgsnd(usrRcv, &msg, sizeof(msg), IPC_NOWAIT);
			}
			
			if(queue3 != NULL)
			{
				table->quant = quant3;
				table->stack[k].stat = 1;
				msg.mtype = table->stack[k].pid;
				strcpy(msg.msg, "RUN");
				msgsnd(usrRcv, &msg, sizeof(msg), IPC_NOWAIT);
			}
		}*/
		

		k++;
		if(k == 17)
		{
			k = 0;
		}
				
		if(clockp->s > generate.s && clockp > generate.n)
		{
			moveClock(&generate, (rand() % 3), 0);
		}
		

		if(dead == 100 && echild == 100 && max == 0)
		{
			fprintf(stderr, "is it making it in here?\n");
			break;
		}

					

	}

	avgTime(&cputime, dead);
	avgTime(&systime, dead);
	avgTime(&wttime, dead);
	fprintf(stderr, "\nAverage times [CPU %d:%d] [SYS %d:%d] [WT %d:%d]\n"
	,cputime.s, cputime.n, systime.s, systime.n, wttime.s, wttime.n);	
	
	clearAll();
	clearMessage();
	fprintf(stderr, "OSS HAS FINISHED\n");
	exit(0);

}

//=====================================================
//Bit Vector Functions
//=====================================================
//make sure to test before implementing to make sure that
//Ptable pointer works.
void initTable()
{
	int i;
	
	for(i = 0; i < MAX; ++i)
	{
		table->stack[i].pid = 0;
	}
}

int findSpot()
{
	int i;
	
	for(i = 0; i < MAX; ++i)
	{
		if(table->stack[i].pid == 0)
		{
			return i;
		}
	}
	return -1;
	
}

void reset(int i)
{
	table->stack[i].pid = 0;
	table->stack[i].sid = 0;
	table->stack[i].quant = 0;
	table->stack[i].p = 0;
	table->stack[i].ct = 0;
	table->stack[i].tcpu.n = 0;
	table->stack[i].tcpu.s = 0;
	table->stack[i].tsys.n = 0;
	table->stack[i].tsys.s = 0;
	table->stack[i].bt.n = 0;
	table->stack[i].bt.s = 0;
	table->stack[i].wt.n = 0;
	table->stack[i].wt.s = 0;
}		
//=====================================================
//Shared Memory Functions
//=====================================================
void initClock()
{
	clockp->s = 0;
	clockp->n = 0;
}

void launch()
{
	clockp = (Clock *)makeSharedMem(clockKey, clockSize, &clockID);
	table = (Ptable *)makeSharedMem(pTableKey, pTableSize, &pTableID);
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
		perror("ERROR: OSS: msgget\n");
		exit(1);
	}

	usrRcv = msgget(usrMsg, PERM);
	if(usrRcv == -1)
	{
		perror("ERROR: OSS: usrRcv\n");
		exit(1);
	}
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

			case PTABLE:
			perror("ERROR: oss: shmget(Ptable)\n");
			break;

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
		
			case PTABLE:
			perror("ERROR: oss: shmat(Ptable)\n");
			break;
		}
		clearAll();
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
		cleanMem(clockp, clockID);
	}
	
	if(pTableID > 0)
	{
		cleanMem(table, pTableID);
	}
}

void clearMessage()
{
	msgctl(osSMsg, IPC_RMID, NULL);
	msgctl(usrRcv, IPC_RMID, NULL);
}

//==============================================================
//Signal Handling
//==============================================================

void timesUp(int sig)
{
	char msg[] = "\nProgram has reached 3 seconds\n";
	int msgSize = sizeof(msg);
	write(STDERR_FILENO, msg, msgSize);
	
	clearAll();
	clearMessage();
	int i;
	for(i = 0; i < MAX; i++)
	{
		if(table->stack[i].pid != 0)
		{
			if(kill(table->stack[i].pid, SIGTERM) == -1)
			{
				perror("ERROR: OSS: SIGTERM(timesUP)\n");
			}
		}
	}
	exit(0);
}

void killAll(int sig)
{
	char msg[] = "\nCaught CTRL+C\n";
	int msgSize = sizeof(msg);
	write(STDERR_FILENO, msg, msgSize);
	
	clearAll();
	clearMessage();
	int i;
	for(i = 0; i < MAX; i++)
	{
		if(table->stack[i].pid != 0)
		{
			if(kill(table->stack[i].pid, SIGTERM) == -1)
			{
				perror("ERROR: OSS: SIGTERM(killAll)\n");
			}
		}
	}
	exit(0);
}
