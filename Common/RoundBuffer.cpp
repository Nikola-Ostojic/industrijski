#include "RoundBuffer.h"

struct node* head = NULL;
struct node* current = NULL;
struct node* tail = NULL;


void Init(void* data, int process, int size) {


	struct node* first = (struct node*)malloc(sizeof(struct node));

	first->data = (char*)data;
	first->nodeID = current_id;
	first->processID = process;
	current_id++;
	first->size_of_data = size;

	head = first;
	tail = first;
	head->next = tail;

}
void addNode(void* data, int process, int size) {

	struct node* new_node = (struct node*)malloc(sizeof(struct node));

	new_node->data = (char*)data;
	new_node->size_of_data = size;
	new_node->nodeID = current_id;
	new_node->processID = process;
	current_id++;

	struct node* temp = (struct node*)malloc(sizeof(struct node));
	temp = tail;
	tail = new_node;
	tail->next = temp;
	head->next = tail;
}
void deleteNode(int id) {

	current = (struct node*)malloc(sizeof(struct node));
	current = tail;

	while (current != NULL) {

		if (current == tail && current->nodeID == id) {
			tail->next = (tail->next)->next;
			tail = tail->next;
			head->next = tail;
			free(current);
		}
		else if (current != tail && current->nodeID == id) {
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


