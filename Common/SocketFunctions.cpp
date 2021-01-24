#pragma comment(lib, "Ws2_32.lib")

#include "SocketFunctions.h"

int Recv(SOCKET s, char *recvbuffer)
{
	int count = 0;
	char *countBuffer = (char *)malloc(sizeof(int));

	while (count < sizeof(int))
	{
		int iResult = Select(s, true);
		iResult = recv(s, countBuffer + count, sizeof(int) - count, 0);
		if (iResult < 0)
		{
			return iResult;
		}
		else if (iResult == 0)
		{

			return iResult;
		}
		else
		{
			count += iResult;
		}
	}

	int msgSize = *((int *)countBuffer);
	free(countBuffer);

	printf("Message size: %d\n", msgSize);

	count = 0;

	while (count < msgSize)
	{
		int iResult = Select(s, true);
		iResult = recv(s, recvbuffer + count, msgSize - count, 0);

		if (iResult < 0)
		{
			return iResult;
		}
		else if (iResult == 0)
		{
			return iResult;
		}
		else
		{
			count += iResult;
		}
	}

	return count;
}

int Send(SOCKET s, char* buff, int len)
{
	char *sendBuff = (char *)malloc(len);
	char *sizeBuff = (char *)malloc(sizeof(int));

	*((int *)sizeBuff) = len;
	
	memcpy(sendBuff, buff, len);
	unsigned int count = 0;
	int iResult;

	while (count < sizeof(int))
	{
		Select(s, false);
		iResult = send(s, sizeBuff + count, sizeof(int) - count, 0);
		if (iResult == SOCKET_ERROR)
		{
			return iResult;
		}
		else
		{
			count += iResult;
		}
	}

	count = 0;

	while ((int)count < len)
	{
		Select(s, false);
		iResult = send(s, sendBuff + count, len - count, 0);
		if (iResult == SOCKET_ERROR)
		{
			return iResult;
		}
		else
		{
			count += iResult;
		}
	}

	free(sendBuff);
	free(sizeBuff);

	return count;
}

int Select(SOCKET s, bool receiving)
{
	int iResult = 0;

	FD_SET set;
	timeval timeVal;

	while (iResult == 0)
	{
		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(s, &set);

		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;

		if (receiving == true) {
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else
		{
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}

		if (iResult == SOCKET_ERROR)
		{
			return iResult;
		}
	}
	return iResult;
}

SOCKET CreateSocketServer(char *port, unsigned long int mode)
{
	int iResult;
	unsigned long int nonBlockingMode = mode;
	SOCKET listenSocket = INVALID_SOCKET;
	addrinfo *resultingAddress = NULL;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	iResult = getaddrinfo(NULL, port, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(resultingAddress);
	iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);

	return listenSocket;
}
SOCKET CreateSocketClient(char *adress, int port, unsigned long int mode)
{
	int iResult;
	unsigned long int nonBlockingMode = mode;
	SOCKET connectSocket = INVALID_SOCKET;
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(adress);
	serverAddress.sin_port = htons(port);
	while (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		/*closesocket(connectSocket);
		WSACleanup();*/
		Sleep(2000);
	}
	printf("Connected to server\n");
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

	return connectSocket;
}