#pragma once
#include "Connect.h"

int Connect(SOCKET socket, SOCKADDR * adresa, int size)
{
	int iResult;
	rMessageHeader buffer;

	iResult = sendto(socket, (char*)&buffer, sizeof(rMessageHeader), 0, adresa, size);

	if (iResult == SOCKET_ERROR) {
		printf("Sendto failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	printf("Poslat zahtev za komunikaciju\n");

	//RecvFrom Server - ocekuje se Accepted
	iResult = recvfrom(socket,
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

		printf("Connected to server");
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
