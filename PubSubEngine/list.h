#pragma once
#ifndef LIST_H_
#define LIST_H_
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<WinSock2.h>


#define DEFAULT_LIST_LIMIT 100

typedef struct ListItem_st
{
	int data;
	struct ListItem_st* next;
}ListItem;

typedef struct List_st
{
	ListItem* head;
	int length;
	int limit;
}List;

List* initList(int limit);

bool addItemToTheEndList(List* list, int data);

int* getAt(List* list, int index);

bool deleteAt(List* list, int index);

bool deleteList(List* list);

#endif