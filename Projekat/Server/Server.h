#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include "../Library/header.h"
#include "Inicijalizacija.h"

#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 100
#define ACCESS_BUFFER_SIZE 64 * 1024
#define IP_ADDRESS_LEN 16

int Close(SOCKET serverSocket);
DWORD WINAPI RecieveMessage(LPVOID param);
int Receive(SOCKET socket, int messageLength, bool* lock, sockaddr_in* clientAddress, rClientMessage* info);