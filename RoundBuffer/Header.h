#pragma once
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct node{
	int id;
	char* data;
	int size_of_data;
	struct node* next;
};

struct node* head = NULL;
struct node* current = NULL;
struct node* tail = NULL;

int current_id = 0;		//ID for nodes

void initRoundBuffer(void* data, int size) {

	struct node* first = (struct node*)malloc(sizeof(struct node));

	first->data = (char*)data;
	first->id = current_id;
	current_id++;
	first->size_of_data = size;
	
	head = first;
	tail = first;
	head->next = tail;
}

//Since it is not specified, nodes will be added at the end of round buffer (beffore tail)
void addNode(void* data, int size) {

	struct node* new_node = (struct node*)malloc(sizeof(struct node));

	new_node->data = (char*)data;
	new_node->size_of_data = size;
	new_node->id = current_id;
	current_id++;

	struct node* temp = (struct node*)malloc(sizeof(struct node));
	temp = tail;
	tail = new_node;
	tail->next = temp;
	head->next = tail;
}

void printWholeBuffer() {

	current = tail;

	while (current != NULL) {
		printf("ID: %d  Size of data: %d  Data: %s", current->id, current->size_of_data, current->data);

		if (current == tail){
			current = tail->next;
		}
		else {
			current = current->next;
		}		
	}

}

//With given ID, if found, node will be deleted
void deleteNode(int id) {

	current = (struct node*)malloc(sizeof(struct node));
	current	= tail;

	while (current != NULL) {

		if (current == tail && current->id == id) {
			tail->next = (tail->next)->next;
			tail = tail->next;
			head->next = tail;
			free(current);
		}
		else if(current != tail && current->id == id){
			struct node* previous = tail;
			while (previous->next != current)
				previous = previous->next;
			previous->next = current->next;
			free(current);
		}
	}

}

void emptyBuffer() {

	current = tail;

	while (current != NULL) {
		struct node* temp = current->next;

		free(current);

		current = temp;
	}
}
