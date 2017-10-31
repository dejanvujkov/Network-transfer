#include "Server.h"

void InitializeSocket(SOCKET * serverSocket, sockaddr_in * serverAddress)
{
	int serverPort = SERVER_PORT;

	serverAddress->sin_family = AF_INET; /*set server address protocol family*/
	serverAddress->sin_addr.s_addr = INADDR_ANY;
	serverAddress->sin_port = htons(serverPort);

	*(serverSocket) = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

	// check if socket creation succeeded
	if (*(serverSocket) == INVALID_SOCKET)
	{
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}