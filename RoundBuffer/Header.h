#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node{
	int id;
	void* data;
	int size_of_data;
	struct node* next;
};

struct node* head = NULL;
struct node* current = NULL;
struct node* tail = NULL;

int current_id = 0;		//ID for nodes

void initRoundBuffer() {


}

//Since it is not specified, nodes will be added at the end of round buffer (beffore tail)
void addNode(void* data, int size) {

}

void printWholeBuffer() {

}

//With given ID, if found, node will be deleted
void deleteNode(int id) {

}

void emptyBuffer() {

}
