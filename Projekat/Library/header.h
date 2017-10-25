#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024

#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>
#include "enums.h"
#include "KruzniBuffer.h"
#include "structs.h"
#include "ThreadHeaders.h"

int Send(rSocket socket, char* data, int len);
int Recv(char* buffer, int len, SOCKET* socket, LPSOCKADDR* clientaddress, int addrLen);
