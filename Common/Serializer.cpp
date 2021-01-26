#include "Serializer.h"

char* Serialize(Node* node)
{
	char* buffer = (char*)malloc(sizeof(Node));
	*((int*)buffer) = node->processId;


	strcpy(buffer + sizeof(int), node->value);

	return buffer;
}

Node* Deserialize(char* buffer)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->processId = *((int*)buffer);
	

	memset(newNode->value, 0, MAX_BUFFER);

	strcpy(newNode->value, buffer + sizeof(int));

	//newNode->value[4] = 0;

	return newNode;
}