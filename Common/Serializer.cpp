#include "Serializer.h"

char* Serialize(Node* node)
{
	char* buffer = (char*)malloc(sizeof(Node));
	*((int*)buffer) = node->processId;
	//*((tm *)(buffer + sizeof(int))) = node->timeStamp;
	//strcpy(buffer + sizeof(tm) + sizeof(int), node->value);
	strcpy(buffer + sizeof(int), node->value);

	return buffer;
}

Node* Deserialize(char* buffer)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->processId = *((int*)buffer);
	//newNode->timeStamp = *((tm *)(buffer + sizeof(int)));
	//strcpy(newNode->value, buffer + sizeof(tm) + sizeof(int));
	strcpy(newNode->value, buffer + sizeof(int));

	return newNode;
}