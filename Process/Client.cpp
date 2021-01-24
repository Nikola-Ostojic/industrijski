


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



enum TipKlijenta {
	GLAVNI = 0,
	POMOCNI = 1
};

typedef struct receiveParameters {
	SOCKET *listenSocket;
	RoundBuffer *rBuffer;
}ReceiveParameters;

bool InitializeWindowsSockets();

int main(int argc, char **argv)
{
	if (InitializeWindowsSockets() == false)
		return 1;

	int iResult;

	RoundBuffer *rBuffer = NULL;
	rBuffer = createRoundBuffer();

	if (rBuffer == NULL)
		return 1;

	TipKlijenta tipKlijenta;
	puts("0 - Glavni proces");
	puts("1 - Pomocni proces");
	scanf("%d", &tipKlijenta);
	getchar();

	if (tipKlijenta == GLAVNI)
	{
		SOCKET connectSocket = CreateSocketClient((char*)HOME_ADDRESS, CONNECT_SOCKET_MAIN, 1);

		int proccesId;

		puts("Unesite id procesa: ");
		scanf("%d", &proccesId);
		getchar();

		char *message = (char *)malloc(MESSAGE_SIZE);

		while (1)
		{
			Node *node = (Node *)(malloc(sizeof(Node)));
			node->processId = proccesId;
			/*
			time_t ttime = time(NULL);
			tm *tm = localtime(&ttime);
			*/
			//cNode->timeStamp = *tm;

			puts("Unesite poruku: ");
			char poruka[MAX_BUFFER];
			scanf("%s", poruka);
			if (strcmp(poruka, "exit") == 0)
			{
				break;
			}

			message = Serialize(node);

			// Send an prepared message with null terminator included
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
				puts("Greska pri popunjavanju queue-a!");
				break;
			}

			printf("Bytes Sent: %ld\n", iResult);
		}

		closesocket(connectSocket);
		free(message);
	}
	else if (tipKlijenta == POMOCNI)
	{
		SOCKET connectSocket = CreateSocketClient((char*)HOME_ADDRESS, CONNECT_SOCKET_OTHER3, 1);
		

		int proccesId;

		puts("Unesite id procesa: ");
		scanf("%d", &proccesId);
		getchar();

		iResult = Select(connectSocket, 0);
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			getchar();
			return 1;
		}
		char b[10];
		itoa(proccesId, b, 10);
		iResult = Send(connectSocket, b, sizeof(b));
		

		SOCKET acceptSocket = accept(connectSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
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
				Node* node = Deserialize(recvbuf);
				if (node->processId == proccesId)
				{
					insertInRBuffer(rBuffer, node);
					puts("Primljena poruka od pomocnog servera.");
				}
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

	deleteRBuffer(rBuffer);
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
