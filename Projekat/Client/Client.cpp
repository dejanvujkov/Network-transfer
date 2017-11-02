#include "Client.h"

int main(int argc, char* argv[])
{
	int buffSize = 300 * 1024 * 1024; //300MB
	int* a;
	int* buffer;
	buffer = (int*)malloc(buffSize);
	if (buffer == NULL)
		return 1;
	//memset(buffer, 77, buffSize);
	for (int i = 1; i <= buffSize / 4;i++)
	{
		buffer[i] = i;
	}
		

	int iResult = 0;

	rSocket* socket = rInitialize();

	rConnect(socket, SERVER_IP_ADDERESS, SERVER_PORT);

	rSend(socket, (char*)buffer, buffSize);

	getchar();

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