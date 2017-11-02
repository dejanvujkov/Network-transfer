#include "Server.h"

int main(int argc, char* argv[])
{
	rSocket * socket = rInitialize();
	
	socket->adresa->sin_family = AF_INET;
	socket->adresa->sin_addr.s_addr = INADDR_ANY;
	socket->adresa->sin_port = htons((u_short)SERVER_PORT);

	bind(socket->socket, (LPSOCKADDR)socket->adresa, socket->sockAddrLen);

	char* data = (char*)malloc(50000000);

	rRecv(socket, data, 50000000);
	printf("aa");
	getchar();
	rSend(socket, data, 50000000);
	getchar();

	return 0;
}