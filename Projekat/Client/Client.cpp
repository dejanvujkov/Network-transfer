#include "Client.h"

// UDP client that uses blocking sockets
int main(int argc, char* argv[])
{
	char* buffer;
	buffer = (char*)malloc(1000000000);
	memset(buffer, 77, 1000000000);

	WSADATA wsaData;
	int iResult = 0;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	rSocket socket;
	socket.addr = SERVER_IP_ADDERESS;
	socket.port = SERVER_PORT;

	iResult = Send(socket, buffer, 1000000000);

	if (iResult == -1) {

		printf("Doslo je do greske prilikom slanja paketa");
		return -1;
	}

	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printf("closesocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	return 0;
}