#pragma once 
#pragma comment(lib, "Ws2_32.lib")
#include "list.h"


List* initList(int limit)
{
	List* list = (List*)malloc(sizeof(list));
	list->head = NULL;
	list->limit = limit;
	list->length = 0;

	return list;
}
bool addItemToTheEndList(List* list, int data)
{
	if (list->length < list->limit)
	{
		ListItem* item = (ListItem*)malloc(sizeof(ListItem));
		item->data = data;
		item->next = NULL;

		ListItem* current = list->head;

		if (current == NULL)
		{
			current = item;
			list->head = current;
			list->length++;
			return true;
		}
		else
		{
			while (current->next != NULL)
			{
				current = current->next;
			}
			current->next = item;
			list->length++;
		}
	}
	else
	{
		printf("Limit of list is reached.\n");
		return false;
	}
}

int* getAt(List* list, int index)
{
	if (list == NULL)
	{
		printf("List uninitialized.\n");
		return NULL;
	}
	if (list->length <= index)
	{
		printf("Index out of bounds.\n");
		return NULL;
	}
	if (index == 0)
	{
		return &(list->head->data);
	}

	ListItem* current = list->head;

	for (int i = 1; i <= index; i++)
	{
		current = current->next;
	}

	return &(current->data);
}

bool deleteAt(List* list, int index)
{
	ListItem* current = list->head;
	ListItem* temp = (ListItem*)malloc(sizeof(ListItem));
	if (list->length < index)
	{
		return false;
	}

	for (int i = 1; i < index - 1; i++)
	{
		if (current->next == NULL)
		{
			return false;
		}
		current = current->next;
	}

	temp = current->next;
	current->next = temp->next;

	list->length--;
	free(temp);

	return true;
}

bool deleteList(List* list)
{
	if (list == NULL)
	{
		return false;
	}
	if (list->length == 0)
	{
		free(list);
		return true;
	}

	ListItem* current = list->head;
	ListItem* temp;

	while (current != NULL)
	{
		temp = current->next;
		free(current);
		current = temp;
	}

	list->head = NULL;

	return true;
}