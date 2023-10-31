#pragma once
#include "hashTable.h"

unsigned int hash(int key)
{

	unsigned int x = (unsigned int)key;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = (x >> 16) ^ x;
	unsigned int retval = x % TABLE_SIZE;
	return retval;

}

HashTable* table_init()
{
	HashTable* table = (HashTable*)malloc(sizeof(HashTable));

	table->items = (TableItem*)malloc(sizeof(TableItem) * TABLE_SIZE);

	for (int i = 0; i < TABLE_SIZE; i++)
	{
		table->items[i].list = NULL;		
	}

	InitializeCriticalSection(&table->cs);
	return table;
}

bool addListToTable(HashTable* table, int key)
{
	int index = hash(key);

	EnterCriticalSection(&table->cs);
	TableItem* item = &(table->items[index]);
	int iResult;


	if (item->list == NULL)
	{
		item->list = initList(DEFAULT_LIST_LIMIT);
		if (item->list == NULL)
		{
			printf("Adding new list to hash table failed.\n");
			LeaveCriticalSection(&table->cs);
			return false;
		}
		
	}
	else
	{
		printf("Specified key already exists in hash table.\n");
		LeaveCriticalSection(&table->cs);
		return false;
	}

	LeaveCriticalSection(&table->cs);
	return true;
}

bool addDataToTable(HashTable* table, int key, int value)
{
	int index = hash(key);

	EnterCriticalSection(&table->cs);

	TableItem* item = &(table->items[index]);
	int iResult;

	if (item->list == NULL)
	{
		printf("Key doesn't exist in hasht table.\n");
		LeaveCriticalSection(&table->cs);
		return false;
	}

	bool retVal = addItemToTheEndList(item->list, value);

	LeaveCriticalSection(&table->cs);
	return retVal;
}

List* getListFromTable(HashTable* table, int key)
{
	int index = hash(key);

	EnterCriticalSection(&table->cs);

	TableItem* item = &(table->items[index]);

	if (item == NULL)
	{
		LeaveCriticalSection(&table->cs);
		return NULL;
	}

	LeaveCriticalSection(&table->cs);

	return item->list;
}

bool tableHasKey(HashTable* table, int key)
{
	int index = hash(key);

	EnterCriticalSection(&table->cs);

	TableItem* item = &(table->items[index]);

	if (item->list == NULL)
	{
		LeaveCriticalSection(&table->cs);
		return false;
	}

	LeaveCriticalSection(&table->cs);

	return true;
}

bool deleteTable(HashTable* table)
{
	if (table == NULL)
	{
		printf("null arguement error\n");
		return false;
	}

	EnterCriticalSection(&table->cs);

	for (int i = 0; i < TABLE_SIZE; i++)
	{
		TableItem* item = &table->items[i];

		if (item->list == NULL)
		{
			continue;
		}
		else
		{
			deleteList(item->list);
		}
	
	}

	LeaveCriticalSection(&table->cs);
	free(table->items);
	DeleteCriticalSection(&table->cs);
	free(table);
	return true;
}

void printTable(HashTable* table)
{
	int j;
	TableItem* item;
	ListItem* current;

	EnterCriticalSection(&table->cs);

	for (int i = 0; i < TABLE_SIZE; i++)
	{
		item = &(table->items[i]);

		if (item == NULL || item->list == NULL || item->list->length == 0)
		{
			continue;
		}

		j = 0;

		current = item->list->head;

		printf("Table row %d:\n", i);
		while (current != NULL)
		{
			printf("Value: %d\n", current->data);
			current = current->next;
		}
		printf("\n");
	}

	LeaveCriticalSection(&table->cs);
}