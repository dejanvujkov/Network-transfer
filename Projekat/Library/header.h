#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024	// 10 MB
#define MAX_UDP_SIZE 64*1024 - 29		// UDP paket - UDP header
#define TIMEOUT_SEC 2 * 1000				// Timeout na socketima
#define INITIAL_CWND 10


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>

#include "enums.h"
#include "KruzniBuffer.h"
#include "ThreadHeaders.h"
#include "structs.h"


rSocket* rInitialize();
int rDeinitialize(rSocket* s);
int rConnect(rSocket* s, char* serverAddress, short port);
int rAccept(rSocket* s, short port);
int rSend(rSocket* s, char* data, int len);
int rRecv(rSocket* s, char* data, int len);