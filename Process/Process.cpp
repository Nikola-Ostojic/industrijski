


#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "../Common/SocketFunctions.h"
#include"../Common/RoundBuffer.h"
#include "../Common/Serializer.h"
#include "../Common/DataNode.h"
#include "../Common/Limitations.h"

#pragma comment(lib, "Ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define MESSAGE_SIZE sizeof(Node)
#define HOME_ADDRESS "127.0.0.1"
#define CONNECT_SOCKET_MAIN 7800
#define CONNECT_SOCKET_OTHER 7801
#define CONNECT_SOCKET_OTHER2 7802
#define CONNECT_SOCKET_OTHER3 7803

bool InitializeWindowsSockets();

DWORD WINAPI callBack(LPVOID lpParam);
void RegisterProcess(int processid); 
void SendData(int processid, char* sendData, int size);

enum TipKlijenta {
	GLAVNI = 0,
	POMOCNI = 1
};


Node inbox[100];
int messagesRecieved = 0;


CRITICAL_SECTION cs;
int ID = 0;
SOCKET connectSocket = INVALID_SOCKET;
bool registered = false;

int main(int argc, char** argv)
{
	if (InitializeWindowsSockets() == false)
		return 1;
	InitializeCriticalSection(&cs);
	int replikator;
	int DEFAULT_PORT;
	RoundBuffer* rBuffer = NULL;
	rBuffer = createRoundBuffer();
	printf("Na koji Replikator se povezujes(1 ili 2):");
	scanf("%d", &replikator);

	if (replikator == 1)
	{
		DEFAULT_PORT = CONNECT_SOCKET_MAIN;
	}
	else if (replikator == 2)
	{
		DEFAULT_PORT = CONNECT_SOCKET_OTHER3;
	}
	else
	{
		printf("Pogresan unos!\n");
		return 1;
	}

	


	connectSocket = CreateSocketClient((char*)HOME_ADDRESS, DEFAULT_PORT, 1);
	int iResult;

	fd_set readfds;
	FD_ZERO(&readfds);
	DWORD funId;
	HANDLE handle;
	handle = CreateThread(NULL, 0, &callBack, &connectSocket, 0, &funId);
	
	char* message = (char*)malloc(MESSAGE_SIZE);
	
	while (1)
	{
		if (ID == 0)
		{
			while (!registered) {
				EnterCriticalSection(&cs);
				printf("\nEnter process ID:\n");
				scanf("%d", &ID);
				LeaveCriticalSection(&cs);
				RegisterProcess(ID);
				Sleep(1000);
			}
		}
		else
		{
			//Node* node = (Node*)malloc(sizeof(Node));
			char enterText[MAX_BUFFER];
			memset(enterText, 0, MAX_BUFFER);

			//node->processId = ID;
			//node->senderType = 0;
			EnterCriticalSection(&cs);
			printf("Enter message:\n");
			LeaveCriticalSection(&cs);
			scanf("%s",enterText);
			if (strcmp(enterText, "exit!") == 0) {
				break;
			}
			SendData(ID, enterText, MESSAGE_SIZE);
			
		}


	}
	

	closesocket(connectSocket);
	free(message);
	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}
DWORD WINAPI callBack(LPVOID lpParam)
{
	SOCKET* connectSocket = (SOCKET*)lpParam;

	int iResult;
	char messageBuffer[MAX_BUFFER];
	Node* newNode;
	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);

		FD_SET(*connectSocket, &readfds);
		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;
		int result = select(0, &readfds, NULL, NULL, &timeVal);

		if (result == 0)
		{
			// vreme za cekanje je isteklo
		}
		else if (result == SOCKET_ERROR)
		{
			//desila se greska prilikom poziva funkcije
		}
		else if (FD_ISSET(*connectSocket, &readfds))
		{
			// rezultat je jednak broju soketa koji su zadovoljili uslov
			iResult = Recv(*connectSocket, messageBuffer);
			if (iResult > 0)
			{	
				//EnterCriticalSection(&cs);
				Node* newNode = Deserialize(messageBuffer);
				char regFailString[] = "---regFail---\n";
				char regOKString[] = "---regOK---\n";
				if (strcmp(newNode->value, regFailString) == 0)
				{
					printf("Vec je registrovan proces sa tim ID-em!\n! Pokusajte ponovo!");
					registered = false;
					FD_CLR(*connectSocket, &readfds);
					ID = 0;
					continue;

				}
				if (strcmp(newNode->value, regOKString) == 0)
				{
					printf("Registracija uspesna! Sada mozete slati poruke replikatoru!\n");
					registered = true;
					FD_CLR(*connectSocket, &readfds);
					continue;
				}

				if (newNode->processId == ID && newNode->senderType == 0)
				{
					free(newNode);
					//LeaveCriticalSection(&cs);
					continue;
				}
				//newNode->timeStamp = *((tm *)(buffer + sizeof(int)));
				//strcpy(newNode->value, buffer + sizeof(tm) + sizeof(int));
				memset(newNode->value, 0, MAX_BUFFER);
				strcpy(newNode->value, messageBuffer + sizeof(int) + sizeof(int));
				
				printf("\n***NOVA PORUKA***\n");
				printf("Value:%s\n", newNode->value);
				printf("ID:%d\n", newNode->processId);
				printf("\n______________\n");
				//LeaveCriticalSection(&cs);
				//free(newNode);
				//messagesRecieved++;
				//free(n);
			}
			else if (iResult == 0)
			{
				// connection was closed gracefully
				printf("Connection with Replicator closed.\n");
				closesocket(*connectSocket);
			}
			else
			{
				// there was an error during recv
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(*connectSocket);
			}
		}
		FD_CLR(*connectSocket, &readfds);
	}

	return 0;
}

void RegisterProcess(int processid)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->processId = processid;
	newNode->senderType = 0;
	char regString[] = "---registrationstring---\n";;
	strcpy(newNode->value, regString);
	char* sendBuff = Serialize(newNode);
	int iResult = Send(connectSocket, sendBuff, MESSAGE_SIZE);
	free(newNode);
}

void SendData(int processid, char* sendData, int size)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	newNode->processId = processid;
	newNode->senderType = 0;
	strcpy(newNode->value, sendData);
	char* sendBuff = Serialize(newNode);
	int iResult = Send(connectSocket, sendBuff, size);
	free(newNode);
}