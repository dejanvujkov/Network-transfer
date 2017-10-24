#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <stdio.h>
#include <conio.h>
#include "../Library/header.h"
#include "../Library/KruzniBuffer.h"

#define SERVER_PORT 15000
#define OUTGOING_BUFFER_SIZE 1024

bool InitializeWindowsSockets();

// for demonstration purposes we will hard code
// local host ip adderss
#define SERVER_IP_ADDERESS "127.0.0.1"

// UDP client that uses blocking sockets
int main(int argc, char* argv[])
{
	
	char* buffer;
	buffer = (char*)malloc(100);
	memset(buffer, 77, 100);



	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}
	

	rSocket socket;
	socket.addr = SERVER_IP_ADDERESS;
	socket.port = SERVER_PORT;


	Send(socket, buffer, 100);


	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("closesocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}


	return 0;
}