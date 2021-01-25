#pragma once
#include <winsock.h>

typedef struct client_st
{
	int ID;
	SOCKET acceptedSocket;

}CLIENT;

typedef struct clientList_st
{
	CLIENT client;
	struct clientList_st *next;
}CLIENT_LIST;

void InitList(CLIENT_LIST** head);
bool addNewClient(CLIENT_LIST** head, CLIENT client);
bool containsClient(CLIENT_LIST** head, CLIENT client);
SOCKET getSocketFromFirst(CLIENT_LIST** head);