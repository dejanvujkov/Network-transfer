#pragma once
#include "Connect.h"

int Connect(SOCKET Socket, SOCKADDR * adresa, int size)
{
	sockaddr_in a = *(sockaddr_in*)adresa;

	int iResult;
	rMessageHeader buffer;
	buffer.id = REQUEST;
	buffer.size = 1000000;

	iResult = sendto(Socket, (char*)&buffer, sizeof(rMessageHeader), 0, adresa, size);

	if (iResult == SOCKET_ERROR) {
		printf("Sendto failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Poslat zahtev za komunikaciju\n");

	//RecvFrom Server - ocekuje se Accepted
	iResult = recvfrom(Socket,
		(char*)&buffer,
		sizeof(rMessageHeader),
		0,
		adresa,
		&size);

	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	if (buffer.id == ACCEPTED) {

		printf("Connected to server\n");
		return 0;
	}
	else if (buffer.id == REJECTED)
	{
		printf("Server rejected connection");
		return -1;
	}
	else
	{
		printf("UNKNOWN ERROR");
		return -1;
	}
}