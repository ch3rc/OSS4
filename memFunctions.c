//==========================================================================
//Date: 	March 22, 2020
//Author:	Cody Hawkins
//Class:	CS4760
//Project:	Assignment 4
//File:		memFunctions.c
//==========================================================================
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

//for OS overhead
void moveClock(Clock *main, int sec, int nan)
{
	while(1)
	{
		if(nan >= 10000000000)
		{
			nan = nan - 1000000000;
			sec++;
		}
		else
		{
			break;
		}
	}

	main->n += nan;
	main->s += sec;
}

//move burst clock
void launchClock(Clock *ptr,int sec, int nan)
{
	ptr->s = sec;
	ptr->n = nan;
	while(ptr->n >= 1000000000)
	{
		ptr->n -= 1000000000;
		ptr->s += 1;
	}
	
}

void avgTime(Clock *ptr, int procs)
{
	long time = (ptr->s * 1000000000) + ptr->n / procs;
	ptr->n = 0;
	ptr->s = 0;
	ptr->n = time;
	while(ptr->n >= 1000000000)
	{
		ptr->n -= 1000000000;
		ptr->s += 1;
	}
}

//If start is NULL creates circular DLL, else puts queue after
//start
void insert(struct Queue **start, int pid)
{
	//create CDLL with first queue
	if(*start == NULL)
	{
		struct Queue *new_queue = (struct Queue *)malloc(sizeof(struct Queue));
		new_queue->pid = pid;
		new_queue->next = new_queue->prev = new_queue;
		*start = new_queue;
		return;
	}
	
	//if queue has head add next queue to the end
	struct Queue *last = (*start)->prev;

	struct Queue *new_queue = (struct Queue *)malloc(sizeof(struct Queue));

	new_queue->pid = pid;

	new_queue->next = *start;

	(*start)->prev = new_queue;

	new_queue->prev = last;

	last->next = new_queue;
}

//delete queues after being ran/terminated
void deleteQ(struct Queue **start, int pid)
{
	if(*start == NULL)
	{
		printf("Queue is empty\n");
		return;
	}

	struct Queue *curr = *start, *prev_1 = NULL;

	while(curr->pid != pid)
	{
		if(curr->next == *start)
		{
			printf("\nQueue does not have pid = %d\n", pid);
			return;
		}

		prev_1 = curr;
		curr = curr->next;
	}

	if(curr->next == *start && prev_1 == NULL)
	{
		(*start) == NULL;
		free(curr);
		return;
	}

	if(curr == *start)
	{
		prev_1 = (*start)->prev;

		*start = (*start)->next;

		prev_1->next = *start;
		(*start)->prev = prev_1;
		free(curr);
	}

	else if(curr->next == *start)
	{
		struct Queue *temp = curr->next;

		prev_1->next = temp;
		temp->prev = prev_1;
		free(curr);
	}
}

//sanity check to make sure queues are being inserted correctly
void display(struct Queue *start)
{
	if(start == NULL)
	{
		printf("\nQueue list is empty\n");
		return;
	}

	struct Queue *temp = start;

	while(temp->next != start)
	{
		printf("%d ", temp->pid);
		temp = temp->next;
	}
	printf("%d ", temp->pid);
}

		
