#include "Client.h"

// UDP client that uses blocking sockets
int main(int argc, char* argv[])
{
	int buffSize = 500000000;
	char* buffer;
	buffer = (char*)malloc(buffSize);
	if (buffer == NULL)
		return 1;
	memset(buffer, 77, buffSize);

	WSADATA wsaData;
	int iResult = 0;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	SOCKET mySocket = INVALID_SOCKET;
	mySocket = socket(AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP);

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDERESS);
	serverAddress.sin_port = htons((u_short)SERVER_PORT);

	int sockAddrLen = sizeof(struct sockaddr);

	//CONNECT
	int SleepTime = 500;
	do {
		iResult = Connect(mySocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress), buffSize);
		if (iResult == -1)
		{
			SleepTime *= 2;
			printf("\nTrying again in %d sec.\n", SleepTime/1000);
			Sleep(SleepTime);
			//getchar();
			//return 1;
		}
	} while (iResult == -1);

	//SEND
	iResult = Send(mySocket, buffer, buffSize, 0, (SOCKADDR*)&serverAddress, sockAddrLen);


	getchar();
	/*rSocket socket;
	socket.addr = SERVER_IP_ADDERESS;
	socket.port = SERVER_PORT;

	iResult = Send(socket, buffer, buffSize);*/

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

	free(buffer);

	return 0;
}