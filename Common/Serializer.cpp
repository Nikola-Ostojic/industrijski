#include "Serializer.h"
#include "Limitations.h"
char* Serialize(Node* node)
{
	char* buffer = (char*)malloc(sizeof(Node));
	*((int*)buffer) = node->processId;
	*((int*)(buffer + sizeof(int))) = node->senderType;
	strcpy(buffer + sizeof(int) + sizeof(int), node->value);
	

	return buffer;
}

Node* Deserialize(char* buffer)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->processId = *((int*)buffer);
	newNode->senderType = *((int*)(buffer + sizeof(int)));
	memset(newNode->value, 0, MAX_BUFFER);
	strcpy(newNode->value, buffer + sizeof(int) + sizeof(int));

	return newNode;
}