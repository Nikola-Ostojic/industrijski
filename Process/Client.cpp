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
#define SERVER_PORT 27016
#define BUFFER_SIZE 256
SOCKET connectSocket = INVALID_SOCKET;

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
	memcpy(dataToSend, stringID, 3);
	memcpy(dataToSend + 3, data, dataSize);
	int iResult = send(connectSocket, dataToSend, (int)strlen(dataToSend), 0);
	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

}

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



	printf("Enter ID: ");
	char IDBuffer[3];
	gets_s(IDBuffer, 3);
	int currentProccessID = atoi(IDBuffer);
	int registered = RegisterService(currentProccessID);

	if (registered == 0)
	{
		printf("Uspesna registracija!\n");
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


	while(true)
	{
		char dataBuffer[BUFFER_SIZE];
		printf("Unesite podatak za slanje");
		gets_s(dataBuffer, BUFFER_SIZE);
		int result = SendData(currentProccessID, (void*)dataBuffer, (int)strlen(dataBuffer));






	}

	// Shutdown the connection since we're done
	iResult = shutdown(connectSocket, SD_BOTH);
	closesocket(connectSocket);
	WSACleanup();
	printf("Ugasen socket");
	return 0;
}