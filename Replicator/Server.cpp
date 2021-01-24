#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include "../Common/SocketFunctions.h"
#include"../Common/RoundBuffer.h"
#include "../Common/Serializer.h"
#include "../Common/DataNode.h"

#pragma comment(lib, "Ws2_32.lib")

#define MESSAGE_SIZE sizeof(Node)
#define LISTEN_SOCKET_MAIN 7800
#define LISTEN_SOCKET_OTHER 7801
#define LISTEN_SOCKET_OTHER2 7802
#define LISTEN_SOCKET_OTHER3 7803
#define HOME_ADDRESS "127.0.0.1"

typedef struct receiveParameters {
	SOCKET* listenSocket;
	RoundBuffer* roundbuffer;
}ReceiveParameters;

typedef struct sendBufferParameters {
	SOCKET* connectSocket;
	RoundBuffer* roundbuffer;
}SendBufferParameters;

enum TipServera {
	GLAVNI = 0,
	POMOCNI = 1
};

bool InitializeWindowsSockets();
DWORD WINAPI SendFromBuffer(LPVOID parameter);
DWORD WINAPI ReceiveMessageClient(LPVOID parameter);

int main(int argc, char** argv)
{
	if (InitializeWindowsSockets() == false)
		return 1;

	int iResult;

	RoundBuffer* rbuffer = NULL;
	rbuffer = createRoundBuffer();

	if (rbuffer == NULL)
		return 1;

	TipServera tipServera;

	puts("0 - Glavni Server");
	puts("1 - Pomocni server");
	scanf("%d", &tipServera);

	if (tipServera == GLAVNI)
	{
#pragma region Slanje round buffera pomocnom serveru
		//SOCKET connectSocket = CreateSocketClient(HOME_ADDRESS, atoi(argv[2]), 1);
		//puts("Unesi adresu pomocnog servera:");
		//char Home_Address[100];
		//scanf("%s", Home_Address);
		SOCKET connectSocket = CreateSocketClient((char*)HOME_ADDRESS, LISTEN_SOCKET_OTHER, 1);

		SendBufferParameters parameters;
		parameters.connectSocket = &connectSocket;
		parameters.roundbuffer = rbuffer;

		//Pravljenje niti za slanje pomocnom serveru
		DWORD dwThreadId;
		CreateThread(NULL, 0, &SendFromBuffer, &parameters, 0, &dwThreadId);
		Sleep(500);
#pragma endregion

#pragma region Primanje poruka od procesa
		char listen_port_main[] = "7800";

		SOCKET listenSocket = CreateSocketServer(listen_port_main, 1);

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
			parameters.roundbuffer = rbuffer;

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
		Node* nodeList[10];
		int numberOdClients = 0;
		char listen_port_other[] = "7801";
		SOCKET listenSocketServer = CreateSocketServer(listen_port_other, 1);

		iResult = listen(listenSocketServer, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocketServer);
			WSACleanup();
			return 1;
		}


		iResult = Select(listenSocketServer, 1);
		if (iResult == SOCKET_ERROR)
		{
			fprintf(stderr, "select failed with error: %ld\n", WSAGetLastError());
			getchar();
			return 1;
		}

		printf("Pomocni server dignut, ceka glavni.\n");
		ReceiveParameters parameters;
		parameters.listenSocket = &listenSocketServer;
		parameters.roundbuffer = rbuffer;

		DWORD dwThreadId;
		CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThreadId);
		Sleep(500);


#pragma endregion

#pragma region Primanje novih procesa
		char listen_port_other3[] = "7803";

		SOCKET listenSocket = CreateSocketServer(listen_port_other3, 1);

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
			parameters.roundbuffer = rbuffer;

			DWORD dwThreadId;
			CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThreadId);
			Sleep(500);
		}

		closesocket(listenSocket);

	}

	deleteRBuffer(rbuffer);
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
	char* message = (char*)malloc(MESSAGE_SIZE);
	SendBufferParameters* parameters = (SendBufferParameters*)parameter;
	RoundBuffer* rbuffer = parameters->roundbuffer;
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

		if (isEmpty(rbuffer) == true)
		{
			puts("Buffer je prazan!");
			Sleep(5000);
			continue;
		}

		Node* node = removeFromRBuffer(rbuffer);
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

		printf("Bytes Sent: %ld\n", iResult);
		Sleep(2000);
	}

	free(message);

	return 0;
}

DWORD WINAPI ReceiveMessageClient(LPVOID parameter)
{
	ReceiveParameters* parameters = (ReceiveParameters*)parameter;

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

	char* recvbuf = (char*)malloc(MESSAGE_SIZE);

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
			printf("Recevied message from client, proces id=%d\n", *(int*)recvbuf);
			Node* node = Deserialize(recvbuf);
			if (insertInRBuffer(parameters->roundbuffer, node) == false)
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