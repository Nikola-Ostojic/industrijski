#define _CRT_NONSTDC_NO_DEPRECATE
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define no_init_all deprecated
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP_ADDRESS "127.0.0.1"
#define SERVER_PORT 27017
#define SERVER_PORT2 27018
#define BUFFER_SIZE 256
SOCKET connectSocket = INVALID_SOCKET;

SOCKET listenSocket = INVALID_SOCKET;

// Socket used for communication with client
SOCKET acceptedSocket = INVALID_SOCKET;

DWORD WINAPI clientRecieve(LPVOID lpParam)
{
	char buffer[BUFFER_SIZE] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;

	while (true)
	{
		if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
		{
			printf("Socket error: %d\n", WSAGetLastError());
			return -1;
		}

		printf("Server:%s", buffer);
		return 1;
	}
}

DWORD WINAPI clientSend(LPVOID lpParam)
{
	char buffer[BUFFER_SIZE] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (true)
	{
		printf("Poruka za slanje:");
		gets_s(buffer);
		if (send(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
			return -1;
	
	}
}


int RegisterService(int ServiceID)
{


	char dataBuffer[3];
	itoa(ServiceID, dataBuffer, 10);
	int iResult = send(connectSocket, dataBuffer, (int)strlen(dataBuffer), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}


	return 0;

}

int SendData(int ServiceID, void* data, int dataSize)
{
	char stringID[3];
	itoa(ServiceID, stringID, 10);
	char* dataToSend = (char*)malloc(3 + dataSize);
	memcpy(dataToSend, stringID, strlen(stringID));
	memcpy(dataToSend + strlen(stringID), data, dataSize);
	int iResult = send(connectSocket, dataToSend, (int)strlen(dataToSend), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

}

int main()
{
	int iResult = 0;

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;								// IPv4 protocol
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	serverAddress.sin_port = htons(SERVER_PORT);					// server port

	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	while (true) {
		DWORD tid;
		HANDLE t1 = CreateThread(NULL, 0, clientSend, &connectSocket, 0, &tid);
	}
	

	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);
	closesocket(connectSocket);
	WSACleanup();
	printf("Ugasen socket");
	return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
/*
// TCP client that use blocking sockets
int main()
{

	int iResult = 0;

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	sockaddr_in clientAddress;
	clientAddress.sin_family = AF_INET;								// IPv4 protocol
	clientAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);	// ip address of server
	clientAddress.sin_port = htons(SERVER_PORT2);


	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;				// IPv4 address family
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);		// Use all available addresses
	serverAddress.sin_port = htons(SERVER_PORT);	// Use specific port		// server port


	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	

	
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}



	printf("Enter ID: ");
	char IDBuffer[3];
	gets_s(IDBuffer, 3);
	int currentProccessID = atoi(IDBuffer);
	int registered = RegisterService(currentProccessID);

	if (registered == 0)
	{
		printf("Uspesna registracija!\n");
		
		listenSocket = socket(AF_INET,      // IPv4 address family
			SOCK_STREAM,  // Stream socket
			IPPROTO_TCP); // TCP protocol
		if (listenSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		int iResult2;
		// Setup the TCP listening socket - bind port number and local address to socket
		iResult2 = bind(listenSocket, (struct sockaddr*) &clientAddress, sizeof(serverAddress));
		if (iResult == SOCKET_ERROR)
		{
			printf("bind failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		iResult2 = listen(listenSocket, SOMAXCONN);
		if (iResult2 == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("Server socket is set to listening mode. Waiting for new connection requests.\n");
		
	}
	else
	{
		printf("Neuspela registracija!\n");
		iResult = shutdown(connectSocket, SD_BOTH);
		closesocket(connectSocket);
		WSACleanup();
		printf("Ugasen socket");
		return 0;
	}


	while (true)
	{
		char dataBuffer[BUFFER_SIZE];
		printf("Unesite podatak za slanje:");
		gets_s(dataBuffer, BUFFER_SIZE);
		int result = SendData(currentProccessID, (void*)dataBuffer, (int)strlen(dataBuffer));

		sockaddr_in clientAddr;

		int clientAddrSize = sizeof(struct sockaddr_in);

		// Accept new connections from clients
		acceptedSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &clientAddrSize);

		// Check if accepted socket is valid
		if (acceptedSocket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("\nNew client request accepted. Client address: %s : %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
		unsigned long mode = 1;
		if (ioctlsocket(acceptedSocket, FIONBIO, &mode) != 0)
		{
			printf("ioctlsocket failed with error %d\n", WSAGetLastError());
			closesocket(acceptedSocket);

			WSACleanup();
			return 0;
		}
		int count = 0;

		while (true)
		{
			fd_set readfds;
			FD_ZERO(&readfds);

			// Add socket to set readfds
			FD_SET(acceptedSocket, &readfds);


			timeval timeVal;
			timeVal.tv_sec = 1;
			timeVal.tv_usec = 0;

			int sResult = select(0, &readfds, NULL, NULL, &timeVal);

			if (sResult == 0)
			{

				Sleep(1000);
			}
			else if (sResult == SOCKET_ERROR)
			{

				printf("select failed with error: %d\n", WSAGetLastError());
				break;
			}
			else
			{
				if (FD_ISSET(acceptedSocket, &readfds))
				{

					DWORD tid;
					HANDLE t1 = CreateThread(NULL, 0, clientRecieve, &connectSocket, 0, &tid);

				}
			}


			


		}






	}
	

	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);
	closesocket(connectSocket);
	WSACleanup();
	printf("Ugasen socket");
	return 0;
}
*/