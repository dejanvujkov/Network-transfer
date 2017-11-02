#include "Server.h"

int main(int argc, char* argv[])
{
	rSocket * socket = rInitialize();
	rAccept(socket, SERVER_PORT);

	int size = 50 * 1024 * 1024;
	char* data = (char*)malloc(size);

	rRecv(socket, data, size);
	
	getchar();

	rDeinitialize(socket);

	free(data);

	return 0;
}