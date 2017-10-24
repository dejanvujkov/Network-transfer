#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>

#define SERVER_PORT 15000
#define SERVER_SLEEP_TIME 100
#define ACCESS_BUFFER_SIZE 1024
#define IP_ADDRESS_LEN 16

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();