#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024	// 10 MB
#define MAX_UDP_SIZE 64*1024 - 29		// UDP paket - UDP header
#define WAIT_FOR_ACK_SEC 2				// Timeout na socketima

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
//int Recv(SOCKET socket, char* buffer, int size, LPSOCKADDR* clientaddress, int addrLen);
int Connect(SOCKET socket, SOCKADDR* adresa, int addrsize, int size);
int Receive(SOCKET socket, int messageLength, bool* lock, sockaddr_in* clientAddress, rClientMessage* info);