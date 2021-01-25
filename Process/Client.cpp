


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
DWORD WINAPI SendFromBuffer(LPVOID parameter);
DWORD WINAPI ReceiveMessageClient(LPVOID parameter);
DWORD WINAPI handleIncomingData(LPVOID lpParam);

typedef struct receiveParameters {
	SOCKET* listenSocket;
	RoundBuffer* roundbuffer;
}ReceiveParameters;

typedef struct sendBufferParameters {
	SOCKET* connectSocket;
	RoundBuffer* roundbuffer;
}SendBufferParameters;


enum TipKlijenta {
	GLAVNI = 0,
	POMOCNI = 1
};




int main(int argc, char** argv)
{
	if (InitializeWindowsSockets() == false)
		return 1;
	int replikator;
	int DEFAULT_PORT;
	RoundBuffer* rBuffer = NULL;
	rBuffer = createRoundBuffer();
	printf("Na koji replikator se povezujes(1 ili 2):");
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

	Node* node = (Node*)malloc(sizeof(Node));


	SOCKET connectSocket = CreateSocketClient((char*)HOME_ADDRESS, DEFAULT_PORT, 1);
	int iResult;

	fd_set readfds;
	FD_ZERO(&readfds);
	DWORD funId;
	HANDLE handle;
	handle = CreateThread(NULL, 0, &handleIncomingData, &connectSocket, 0, &funId);
	int ID = 0;
	char* message = (char*)malloc(MESSAGE_SIZE);
	
	while (1)
	{
		if (ID == 0)
		{
			printf("Enter process ID:");
			scanf("%d", &ID);
		}
		else
		{
			Node* node = (Node*)malloc(sizeof(Node));
			node->processId = ID;
			printf("Enter message:");
			scanf("%s", node->value);

			message = Serialize(node);
			iResult = Send(connectSocket, message, MESSAGE_SIZE);

			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(connectSocket);
				WSACleanup();
				return 1;
			}
			if (insertInRBuffer(rBuffer, node) == false)
			{

				printf("Greska pri popunjavanju buffera!\n");
				break;
			}

			printf("Bytes Sent: %ld\n", iResult);
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
DWORD WINAPI handleIncomingData(LPVOID lpParam)
{
	SOCKET* connectSocket = (SOCKET*)lpParam;

	int iResult;
	char messageBuffer[MAX_BUFFER];

	while (true)
	{
		fd_set readfds;
		FD_ZERO(&readfds);

		FD_SET(*connectSocket, &readfds);
		timeval timeVal;
		timeVal.tv_sec = 2;
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
			iResult = recv(*connectSocket, messageBuffer, MESSAGE_SIZE, 0);
			if (iResult > 0)
			{
				printf("%s", messageBuffer);
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



