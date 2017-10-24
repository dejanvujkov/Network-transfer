#include <WinSock2.h>
#include "enums.h"
#include "structs.h"
#include "KruzniBuffer.h"
#include <stdlib.h>
#include <stdio.h>

int Send(rSocket socket, char* data, int len);
int Recv(char* buffer, int len, SOCKET* socket, LPSOCKADDR* clientaddress, int addrLen);

