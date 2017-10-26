#pragma once
#include "Connect.h"

int KonektujSe(rHelper *h, int len) {

	rMessageHeader header;
	header.id = REQUEST;
	header.size = len;

	int iResult;

	iResult = sendto(*(h->socket),
		(char*)&header,
		sizeof(rMessageHeader),
		0,
		(LPSOCKADDR)(h->adresa),
		h->sockAddrLen);

	if (iResult == SOCKET_ERROR) {
		printf("Sendto failed with error: %d\n", WSAGetLastError());
		closesocket(*(h->socket));
		WSACleanup();
		return 1;
	}

	printf("Poslat zahtev za komunikaciju\n");

	//RecvFrom Server - ocekuje se Accepted
	iResult = recvfrom(*(h->socket),
		(char*)&header,
		sizeof(rMessageHeader),
		0,
		(LPSOCKADDR)(h->adresa),
		&h->sockAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	if (header.id == ACCEPTED) {

		printf("Connected to server");
		h->state = CONNECTED;
	}
	else if (header.id == REJECTED)
	{
		printf("Server rejected connection");
		h->state = DISCONNECTED;
	}

	return 0;
}