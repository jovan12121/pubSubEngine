#pragma once
#ifndef HASHTABLE_H_
#define HASHTABLE_H_
#include <string.h>
#include "list.h"

#define TABLE_SIZE 100

typedef struct
{
    int key;
    List* list;
} TableItem;

typedef struct
{
    CRITICAL_SECTION cs;
    TableItem* items;
} HashTable;

unsigned int hash(int key);

HashTable* table_init();

bool addListToTable(HashTable* table, int key);

bool addDataToTable(HashTable* table, int key, int value);

List* getListFromTable(HashTable* table, int key);

bool tableHasKey(HashTable* table, int key);

bool deleteTable(HashTable* table);

void printTable(HashTable* table);

#endif