#pragma once
#include "Server.h"

void InitializeSocket(SOCKET* serverSocket, sockaddr_in *serverAddress);
bool InitializeWindowsSockets();