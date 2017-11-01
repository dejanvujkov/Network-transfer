#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024	// 10 MB
#define MAX_UDP_SIZE 64*1024 - 29		// UDP paket - UDP header
#define TIMEOUT_SEC 2 * 1000				// Timeout na socketima
#define INITIAL_CWND 10


//cl

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


rSocket* rInitialize();
int rDeinitialize(rSocket* s);
int rConnect(rSocket* s, char* serverAddress, short port);
int rAccept(rSocket* s);
int rSend(rSocket* s, char* data, int len);
int rRecv(rSocket* s, char* data, int len);


/////////////
//
////int Send(rSocket socket, char* data, int len);
//int Send(SOCKET socket, char* buffer, int size, int flag, sockaddr* adresa, int tolen);
////int Recv(SOCKET socket, char* buffer, int size, LPSOCKADDR* clientaddress, int addrLen);
//int Connect(SOCKET socket, SOCKADDR* adresa, int addrsize, int size);
//int Receive(SOCKET socket, int messageLength, bool* lock, sockaddr_in* clientAddress, rClientMessage* info);
//void Initialize(rHelper* h, SOCKET* socket, char* buffer, int size, sockaddr* adresa, int tolen);