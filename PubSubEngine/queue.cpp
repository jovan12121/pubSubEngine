#include "Queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGE 512


void InitializeQueue(Queue** queue)
{
	*queue = (Queue*)malloc(sizeof(Queue));
	(*queue)->count = 0;
	(*queue)->front = NULL;
	(*queue)->rear = NULL;

	InitializeCriticalSection(&((*queue)->queue_cs));
}

int IsEmptyQueue(Queue* queue)
{
	return queue->count == 0;
}

void Enqueue(Queue** queue, int topicId, char* message)
{

	EnterCriticalSection(&((*queue)->queue_cs));

	if ((*queue)->count < QUEUE_MAX_SIZE)
	{
		Queue_node* temp;
		temp = (Queue_node*)malloc(sizeof(Queue_node));


		memset(temp->message, 0, MAX_MESSAGE);
		memcpy(temp->message, message, strlen(message));		

		temp->topicId = topicId;
		temp->next = NULL;

		if (!IsEmptyQueue(*queue))
		{
			(*queue)->rear->next = temp;
			(*queue)->rear = temp;
		}
		else
		{
			(*queue)->front = temp;
			(*queue)->rear = temp;
		}
		(*queue)->count++;
	}


	LeaveCriticalSection(&((*queue)->queue_cs));
}



void Dequeue(Queue** queue, char* message, int* topicId)
{
	Queue_node* temp;

	EnterCriticalSection(&((*queue)->queue_cs));


	strcpy_s(message, 512, (*queue)->front->message);

	(*topicId) = (*queue)->front->topicId;



	temp = (*queue)->front;
	(*queue)->front = (*queue)->front->next;
	(*queue)->count--;

	free(temp);

	LeaveCriticalSection(&((*queue)->queue_cs));


}

void DeleteQueueCirticalSection(Queue** queue)
{
	DeleteCriticalSection(&((*queue)->queue_cs));
}

void FreeQueue(Queue** queue)
{

	Queue_node* temp;

	EnterCriticalSection(&((*queue)->queue_cs));



	while ((*queue)->front != NULL)
	{

		temp = (*queue)->front;
		(*queue)->front = (*queue)->front->next;
		(*queue)->count--;

		free(temp);
	}

	(*queue)->count = 0;
	(*queue)->front = NULL;
	(*queue)->rear = NULL;

	LeaveCriticalSection(&((*queue)->queue_cs));
	DeleteCriticalSection(&((*queue)->queue_cs));

	free(*queue);
	*queue = NULL;

}