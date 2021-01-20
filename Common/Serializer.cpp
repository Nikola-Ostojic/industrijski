#include "Serializer.h"

char* Serialize(node* Node)
{
	char *buffer = (char*)malloc(sizeof(Node));
	*((int *)buffer) = Node->processID;	
	strcpy(buffer + sizeof(int),Node->data);

	return buffer;
}

node* Deserialize(char *buffer)
{
	node *newNode = (node*)malloc(sizeof(node));
	newNode->processID = *((int *)buffer);	
	strcpy(newNode->data, buffer +sizeof(int));

	return newNode;
}