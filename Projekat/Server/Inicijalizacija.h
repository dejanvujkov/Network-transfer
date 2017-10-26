#pragma once
#include "Server.h"
#include "../Library/header.h"

void InitializeSocket(SOCKET* serverSocket, sockaddr_in *serverAddress);
bool InitializeWindowsSockets();