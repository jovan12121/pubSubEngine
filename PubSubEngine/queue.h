#pragma once
#ifndef QUEUE_H_
#define QUEUE_H_

#define QUEUE_MAX_SIZE 10000000
#define MAX_MESSAGE_LEN 512
#define MAX_TITLE_LEN 128

#include <ws2tcpip.h>

typedef struct Queue_node
{
	int topicId;
	char message[MAX_MESSAGE_LEN];

	Queue_node* next;

}Queue_node;


typedef struct Queue
{
	int count;
	Queue_node* front;
	Queue_node* rear;
	CRITICAL_SECTION queue_cs;


}Queue;


void InitializeQueue(Queue** queue);

int IsEmptyQueue(Queue* queue);

void Enqueue(Queue** queue, int topicId, char* message);

void Dequeue(Queue** queue, char* message, int* topicId);

void DeleteQueueCirticalSection(Queue** queue);

void FreeQueue(Queue** queue);

#endif