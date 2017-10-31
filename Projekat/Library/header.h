#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024	// 10 MB
// Korisna velicina UDP paketa
#define MAX_UDP_SIZE 64*1024 - 29
#define WAIT_FOR_ACK_SEC 5

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>

#include "enums.h"
#include "KruzniBuffer.h"
#include "ThreadHeaders.h"
#include "Connect.h"
#include "structs.h"
#include "Initialize.h"

//int Send(rSocket socket, char* data, int len);
int Send(SOCKET socket, char* buffer, int size, int flag, sockaddr* adresa, int tolen);
int Recv(SOCKET socket, char* buffer, int size, LPSOCKADDR* clientaddress, int addrLen);
int Connect(SOCKET socket, SOCKADDR* adresa, int addrsize, int size);