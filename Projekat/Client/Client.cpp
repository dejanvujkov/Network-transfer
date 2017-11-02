#include "Client.h"

int main(int argc, char* argv[])
{
	int buffSize = 50 * 1024 * 1024; //300MB
	int* a;
	int* buffer;
	buffer = (int*)malloc(buffSize);
	if (buffer == NULL)
		return 1;
	memset(buffer, 77, buffSize);

	rSocket* socket = rInitialize();
	rConnect(socket, SERVER_IP_ADDERESS, SERVER_PORT);
	rSend(socket, (char*)buffer, buffSize);
	
	getchar();

	rDeinitialize(socket);		

	free(buffer);

	return 0;
}