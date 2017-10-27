#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024	// 10 MB
// Korisna velicina UDP paketa
#define MAX_UDP_SIZE 64*1024 - 29

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>

#include "enums.h"
#include "KruzniBuffer.h"
#include "ThreadHeaders.h"
#include "Initialize.h"
#include "Connect.h"
#include "structs.h"

//int Send(rSocket socket, char* data, int len);
int Send2(SOCKET socket, char* buffer, int size, int flag, sockaddr* adresa, int tolen);
int Recv(char* buffer, int len, SOCKET* socket, LPSOCKADDR* clientaddress, int addrLen);
