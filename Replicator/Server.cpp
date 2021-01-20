#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "../Common/SocketFunctions.h"
#include"../Common/RoundBuffer.h"
#include "../Common/Serializer.h"
#include "../Common/ClientNode.h"

#pragma comment(lib, "Ws2_32.lib")

#define MESSAGE_SIZE sizeof(CNode)

typedef struct receiveParameters {
	SOCKET *listenSocket;
	node *Node;
}ReceiveParameters;

typedef struct sendBufferParameters {
	SOCKET *connectSocket;
	node *Node;
}SendBufferParameters;

enum TipServera {
	GLAVNI = 0,
	POMOCNI = 1
};

bool InitializeWindowsSockets();
DWORD WINAPI SendFromBuffer(LPVOID parameter);
DWORD WINAPI ReceiveMessageClient(LPVOID parameter);

int main(int argc, char **argv)
{
	if (InitializeWindowsSockets() == false)
		return 1;

	int iResult;

	//node *node = NULL;
	queue = createQueue();

	if (queue == NULL)
		return 1;

	TipServera tipServera;

	puts("0 - Glavni Server");
	puts("1 - Pomocni server");
	scanf("%d", &tipServera);

	if (tipServera == GLAVNI)
	{
#pragma region Slanje queue-a pomocnom serveru
		SOCKET connectSocket = CreateSocketClient(argv[1], atoi(argv[2]), 1);

		SendBufferParameters parameters;
		parameters.connectSocket = &connectSocket;
		parameters.queue = queue;

		//Pravljenje niti za slanje pomocnom serveru
		DWORD dwThreadId;
		CreateThread(NULL, 0, &SendFromBuffer, &parameters, 0, &dwThreadId);
		Sleep(500);
#pragma endregion

#pragma region Primanje poruka od procesa
		SOCKET listenSocket = CreateSocketServer(argv[3], 1);

		iResult = listen(listenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("Glavni server pokrenut, ceka poruke procesa.\n");

		while (1)
		{
			iResult = Select(listenSocket, 1);
			if (iResult == SOCKET_ERROR)
			{
				fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
				getchar();
				return 1;
			}

			ReceiveParameters parameters;
			parameters.listenSocket = &listenSocket;
			parameters.queue = queue;

			DWORD dwThreadId;
			CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThreadId);
			Sleep(500);
		}

		closesocket(listenSocket);
#pragma endregion
	}
	else if (tipServera == POMOCNI)
	{
#pragma region Primanje poruka od glavnog servera
		SOCKET listenSocketServer = CreateSocketServer(argv[1], 1);

		iResult = listen(listenSocketServer, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocketServer);
			WSACleanup();
			return 1;
		}

		printf("Pomocni server dignut, ceka glavni.\n");

		iResult = Select(listenSocketServer, 1);
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			getchar();
			return 1;
		}

		ReceiveParameters parameters;
		parameters.listenSocket = &listenSocketServer;
		parameters.queue = queue;

		DWORD dwThreadId;
		CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThreadId);
		Sleep(500);
#pragma endregion

#pragma region Slanje poruka procesima
		SOCKET listenSocketClients = CreateSocketServer(argv[2], 1);

		iResult = listen(listenSocketClients, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocketClients);
			WSACleanup();
			return 1;
		}

		printf("Pomocni server dignut, ceka procese.\n");

		while (1)
		{
			iResult = Select(listenSocketClients, 1);
			if (iResult == SOCKET_ERROR)
			{
				fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
				getchar();
				return 1;
			}

			ReceiveParameters parameters;
			parameters.listenSocket = &listenSocketClients;
			parameters.queue = queue;

			DWORD dwThreadId;
			CreateThread(NULL, 0, &SendFromBuffer, &parameters, 0, &dwThreadId);
			Sleep(500);
		}

		closesocket(listenSocketClients);
#pragma endregion
	}

	deleteQueue(queue);
	WSACleanup();

	getchar();

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

DWORD WINAPI SendFromBuffer(LPVOID parameter)
{
	char *message = (char *)malloc(MESSAGE_SIZE);
	SendBufferParameters *parameters = (SendBufferParameters *)parameter;
	Queue *queue = parameters->queue;
	SOCKET connectSocket = *(parameters->connectSocket);

	int iResult;

	while (1)
	{
		if (_kbhit() != false)
		{
			char c = getchar();
			if (c == 27)
				break;
		}

		if (isEmpty(queue) == true)
		{
			puts("Queue je prazan!");
			Sleep(5000);
			continue;
		}

		CNode *cNode = removeFromQueue(queue);
		message = Serialize(cNode);

		// Send an prepared message with null terminator included
		iResult = Send(connectSocket, message, MESSAGE_SIZE);

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
			return 1;
		}

		printf("Bytes Sent: %ld\n", iResult);
		Sleep(2000);
	}

	free(message);

	return 0;
}

DWORD WINAPI ReceiveMessageClient(LPVOID parameter)
{
	ReceiveParameters *parameters = (ReceiveParameters *)parameter;

	SOCKET acceptSocket = accept(*(parameters->listenSocket), NULL, NULL);
	if (acceptSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(*(parameters->listenSocket));
		WSACleanup();
		return 1;
	}

	unsigned long int nonBlockingMode = 1;
	int iResult = ioctlsocket(acceptSocket, FIONBIO, &nonBlockingMode);
	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	char *recvbuf = (char*)malloc(MESSAGE_SIZE);

	iResult = Select(acceptSocket, 0);
	if (iResult == -1)
	{
		fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
		closesocket(acceptSocket);
		WSACleanup();
		return 1;
	}

	do
	{
		memset(recvbuf, 0, MESSAGE_SIZE);
		// Receive data until the client shuts down the connection
		iResult = Recv(acceptSocket, recvbuf);
		if (iResult > 0)
		{
			printf("Recevied message from client, proces id=%d\n", *(int *)recvbuf);
			CNode *cNode = Deserialize(recvbuf);
			if (insertInQueue(parameters->queue, cNode) == false)
				puts("Error inserting in queue");
		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client closed.\n");
			closesocket(acceptSocket);
		}
		else
		{
			// there was an error during recv
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				iResult = 1;
				continue;
			}
			else
			{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(acceptSocket);
			}
		}
	} while (iResult > 0);

	free(recvbuf);

	return 0;
}