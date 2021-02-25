#pragma once
#pragma warning(disable:4996) 
#ifndef SocketFunctions_H
#define SocketFunctions_H

#include <ws2tcpip.h>
#include <WinSock2.h>
#include <stdlib.h>
#include <stdio.h>

int Recv(SOCKET s, char *recvbuffer);
int Select(SOCKET s, bool receiving);
int Send(SOCKET s, char* buff, int len);
SOCKET CreateSocketServer(char *port, unsigned long int mode);
SOCKET CreateSocketClient(char *adress, int port, unsigned long int mode);

#endif // !SocketFunctions_H
