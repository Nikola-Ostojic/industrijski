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
#include "../Common/Processes.h"

#pragma comment(lib, "Ws2_32.lib")

#define MESSAGE_SIZE sizeof(Node)
#define LISTEN_SOCKET_MAIN 7800
#define LISTEN_SOCKET_OTHER 7801
#define LISTEN_SOCKET_OTHER2 7802
#define LISTEN_SOCKET_OTHER3 7803
#define FORWARD_TO_MAIN 10001
#define HOME_ADDRESS "127.0.0.1"

typedef struct receiveParameters {
	SOCKET* listenSocket;
	RoundBuffer* roundbuffer;
}ReceiveParameters;

typedef struct sendBufferParameters {
	SOCKET* connectSocket;
	RoundBuffer* roundbuffer;
}SendBufferParameters;

typedef struct forwardToRStructure
{
	SOCKET* listenSocket;
	char* buffer;
} FORWARD_STRUCT;

enum TipServera {
	GLAVNI = 0,
	POMOCNI = 1
};
CRITICAL_SECTION c;
bool InitializeWindowsSockets();
DWORD WINAPI SendFromBuffer(LPVOID parameter);
DWORD WINAPI ReceiveMessageClient(LPVOID parameter);
/// 
DWORD WINAPI handleResponseFromReplicator(LPVOID prameters);
DWORD WINAPI forwardMessageToReplicator(LPVOID parameters);
DWORD WINAPI recMesFromR2(LPVOID parameter);
/// 
int server;


SOCKET acceptSockets[5];
SOCKET acceptSockets2[5];
SOCKET R2R1AcceptSocket = INVALID_SOCKET;


int clients = 0;
int clients2 = 0;
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
	server = tipServera;

	if (tipServera == GLAVNI)
	{
#pragma region Slanje round buffera pomocnom serveru
		//socket za povezivanje glavnog replikatora na pomocni
		SOCKET connectSocket = CreateSocketClient((char*)HOME_ADDRESS, LISTEN_SOCKET_OTHER, 1);

		SendBufferParameters parameters;
		parameters.connectSocket = &connectSocket; //metodi SendFromBuffer se salje struktura SendBufferParameters
		parameters.roundbuffer = rbuffer;

		//Thread za povezivanje glavnog replikatora na pomocni
		DWORD dwThreadId;
		CreateThread(NULL, 0, &SendFromBuffer, &parameters, 0, &dwThreadId); //pravljenje threada i slanje strukture SendBufferParameters
		Sleep(500);


		//Thread za slusanje, glavni replikator slusa na istom socketu na kojem se povezao na pomocni replikator
		DWORD dwThread12Id;
		CreateThread(NULL, 0, &handleResponseFromReplicator, &connectSocket, 0, &dwThread12Id); //ceka odgovor pomocnog replikatora
		Sleep(500);


#pragma endregion

#pragma region Primanje poruka od procesa
		char listen_port_main[] = "7800";
		//socket za povezivanje novih procesa na glavni replikator
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

			ReceiveParameters parameters; //Struktura RecieveParameters se prosledjuje threadu koji koristi metodi RecieveMessageClient
			parameters.listenSocket = &listenSocket;//strukturi se prosledjuje listenSocket koji osluskuje za nove klijente
			parameters.roundbuffer = rbuffer;//i round buffer

			DWORD dwThread3Id;//Pokretanje threada koji osluskuje za povezivanje klijenata
			CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThread3Id);//i prosledjivanje parametara metodi
			Sleep(500);
		}

		closesocket(listenSocket);
		closesocket(connectSocket);
#pragma endregion
	}
	else if (tipServera == POMOCNI)
	{
#pragma region Primanje poruka od glavnog servera
		
		char listen_port_other[] = "7801";
		//Pravljenje socketa koji osluskuje za povezivanje glavnog replikatora na pomocni
		SOCKET listenSocketServer = CreateSocketServer(listen_port_other, 1);//Na ovaj socket se povezuje glavni replikator

		iResult = listen(listenSocketServer, SOMAXCONN);//slusanje na tom socketu
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
		ReceiveParameters parameters;//strukutra za prosledjivanje threadu koji ceka glavni replikator
		parameters.listenSocket = &listenSocketServer;//salje se socket na kojem slusa i buffer
		parameters.roundbuffer = rbuffer;

		DWORD dwThreadId; //Replikator pokrece metodu RecieveMessageClient koja cita poruku i upisuje je u buffer
		CreateThread(NULL, 0, &ReceiveMessageClient, &parameters, 0, &dwThreadId);//pokretanje threada za slusanje, na koji se povezuje glavni replikator
		Sleep(500);


		SendBufferParameters parameters2; //pravim jos jedan thread, koji bi trebao iz buffera da salje poruke na accept socket
		parameters2.connectSocket = &R2R1AcceptSocket;//acceptsocket sam sacuvao u metodi RecieveMessageClient kada se glavni replikator
		parameters2.roundbuffer = rbuffer;												                         //povezao na pomocni

		DWORD dwThreadId12;
		CreateThread(NULL, 0, &SendFromBuffer, &parameters2, 0, &dwThreadId12);//pri primljenoj poruci od procesa na pomocnom replikatoru, ona bi trebala
		Sleep(500);																									//da bude upisana u buffer
																										//a zatim da je ova metoda procita i posalje
		closesocket(listenSocketServer);															//na accept socket(ka glavnom replikatoru)				
		//closesocket(R2R1AcceptSocket);
#pragma endregion

#pragma region Primanje novih procesa
		char listen_port_other3[] = "7803"; //slusanje za povezivanje procesa na pomocni replikator
		for (int i = 0; i < clients; i++)
		{
			acceptSockets[i] = INVALID_SOCKET; //lista accept socketa se cuva kako bi poruka mogla da se prosledi svim povezanim procesima
		}																					//na pomocni replikator

		SOCKET listenSocket = CreateSocketServer(listen_port_other3, 1);//pravljenje socketa za povezivanje procesa na pomocni replikator

		iResult = listen(listenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("Pomocni server pokrenut, ceka poruke procesa.\n");
		do
		{
			acceptSockets[clients] = accept(listenSocket, NULL, NULL);//u promenljivoj clients se cuva broj povezanih procesa, tako da kad se
																	  //poveze novi proces, ona se povecava
			iResult = Select(acceptSockets[clients], 1);
			if (iResult > 0)			//povezao se novi proces, iResult je vece od 0
			{
				
				
				printf("Novi proces se povezao na replikatora 2!\n");
				clients++;									
				Node* testNode = (Node*)malloc(sizeof(Node));		//testNode pravim jer nije implementirano citanje/deserijalizovanje/serijalizovanje
				testNode->processId = 89;							//primljenog paketa, pa saljem samo ovo
				

				insertInRBuffer(rbuffer, testNode);		//poruka se stavlja u buffer, kako bi je procitao proces koji proverava buffer, i prosledio 
				free(testNode);																						//glavnom replikatoru
				printf("Uspesno poslata poruka replikatoru\n");//ispise se i kad nije tacno
				
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			}

			//closesocket(acceptSocket);
		} while (1);

		
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
		CLIENT c;
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
			
			if (server == 0) {
				if (insertInRBuffer(parameters->roundbuffer, node) == false)
					puts("Error inserting in queue");
			}
			else if (server == 1)
			{
				R2R1AcceptSocket = acceptSocket;
				for (int i = 0; i < clients; i++)
				{
					iResult = Send(acceptSockets[i], recvbuf, sizeof(recvbuf));
					if (iResult == SOCKET_ERROR)
					{
						printf("send failed with error: %d\nPoruka nije poslata\n", WSAGetLastError());
						closesocket(acceptSockets[i]);
						WSACleanup();
						return 1;
					}
					printf("Uspesno poslata poruka\n");
				}
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
///////////////////////////////////////////////////////////////
DWORD WINAPI handleResponseFromReplicator(LPVOID parameters)
{
	SOCKET* connectSocket = (SOCKET*)parameters;

	int iResult;
	char messageBuffer[MAX_BUFFER];
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
				printf("Message from repl2:%s\n", messageBuffer);
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
//ne koristim nigde
DWORD WINAPI recMesFromR2(LPVOID parameter)
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


//ne koristim nigde
DWORD WINAPI forwardMessageToReplicator(LPVOID parameters)
{
	FORWARD_STRUCT* forward = (FORWARD_STRUCT*)parameters;
	SOCKET* listenSocket = (SOCKET*)forward->listenSocket;
	char* messageBuffer = forward->buffer;

	int iResult;
	SOCKET acceptSocket = accept(*listenSocket, NULL, NULL);
	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(acceptSocket, FIONBIO, &nonBlockingMode);
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
	iResult = Send(acceptSocket, messageBuffer, MESSAGE_SIZE);
	
	return 0;
}