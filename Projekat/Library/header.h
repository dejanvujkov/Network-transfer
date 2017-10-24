#include "enums.h"
#include "KruzniBuffer.h"
#include "structs.h"
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#include <windows.h>

int Send(rSocket socket, char* data, int len);
int Recv(char* buffer, int len, SOCKET* socket, LPSOCKADDR* clientaddress, int addrLen);

