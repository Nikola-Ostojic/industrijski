#include "Processes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>


CRITICAL_SECTION cs;
void InitList(CLIENT_LIST** head)
{
	InitializeCriticalSectionAndSpinCount(&cs, 0x80000400);
	EnterCriticalSection(&cs);
	head = NULL;
	LeaveCriticalSection(&cs);
}
bool containsClient(CLIENT_LIST** head, CLIENT client)
{
	CLIENT_LIST *current = *head;
	while (current != NULL)
	{
		int IDs = current->client.ID;
		if (IDs == client.ID)
		{
			return true;
		}
		EnterCriticalSection(&cs);
		current = current->next;
		LeaveCriticalSection(&cs);
	}
	return false;
}

bool addNewClient(CLIENT_LIST** head, CLIENT client)
{
	if (containsClient(head, client))
	{
		return false;
	}
	CLIENT_LIST *current = *head;
	CLIENT_LIST *newMember = (CLIENT_LIST*)malloc(sizeof(CLIENT_LIST));

	newMember->client = client;
	newMember->next = NULL;
	if (current == NULL)
	{
		EnterCriticalSection(&cs);
		*head = newMember;
		LeaveCriticalSection(&cs);
		return true;
	}
	while (current->next != NULL)
	{
		current = current->next;
	}
	EnterCriticalSection(&cs);
	current->client = client;
	current->next = NULL;
	LeaveCriticalSection(&cs);
	return true;
}

SOCKET getSocketFromFirst(CLIENT_LIST** head)
{
	CLIENT_LIST* current = *head;
	SOCKET acceptSocket = INVALID_SOCKET;
	if (current == NULL)
	{
		return acceptSocket;
	}
	EnterCriticalSection(&cs);
	acceptSocket =  current->client.acceptedSocket;
	*head = current->next;
	free(current);
	LeaveCriticalSection(&cs);
	return acceptSocket;

}