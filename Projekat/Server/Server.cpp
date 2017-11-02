#include "Server.h"

int main(int argc, char* argv[])
{
	rSocket * socket = rInitialize();

	rAccept(socket, SERVER_PORT);

	int size = 300 * 1024 * 1024;
	char* data = (char*)malloc(size);

	rRecv(socket, data, size);

	getchar();

	return 0;
}