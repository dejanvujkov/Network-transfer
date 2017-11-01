#pragma once

void InitializeSocket(SOCKET* serverSocket, sockaddr_in *serverAddress);
bool InitializeWindowsSockets();