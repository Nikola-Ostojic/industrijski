#pragma once
#include <stdio.h>
#include "Limitations.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node {
	int processID;
	int nodeID;
	char* data;
	int size_of_data;
	struct node* next;
};

struct node* head = NULL;
struct node* current = NULL;
struct node* tail = NULL;
int current_id = 0;

void Init(void* data, int size);
void addNode(void* data, int size);
void deleteNode(int id);
void emptyBuffer();