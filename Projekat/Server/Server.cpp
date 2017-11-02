#include "Server.h"

int main(int argc, char* argv[])
{
	int size = 50 * 1024 * 1024;
	char* data = (char*)malloc(size);
	
	rSocket * socket = rInitialize();
	printf("\nServer inicijalizovan");
	rAccept(socket, SERVER_PORT);
	printf("\nServer pokrenut");
	
	rRecv(socket, data, size);
	
	getchar();

	rDeinitialize(socket);
	free(data);

	return 0;
}