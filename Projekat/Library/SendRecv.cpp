#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "header.h"
#include <stdlib.h>
#include <stdio.h>
#include "enums.h"

int Send(rSocket sock, char* data, int len)
{
	
	/** INICIJALIZACIJA **/
	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));
	h->slider = 0;
	h->state = DISCONNECTED;

	h->cwnd = 10;			// Inicijalno 10
	h->ssthresh = 0;
	h->recv = 1000;			// Inicijalno mora biti veci od cwnd
	h->slowstart = true;
	/** INICIJALIZACIJA **/

	int iResult = 0;

	/** INIT Client **/
	int sockAddrLen = sizeof(struct sockaddr);
	
	sockaddr_in clientAddress;
	memset((char*)&clientAddress, 0, sizeof(clientAddress));
	clientAddress.sin_family = AF_INET;
	clientAddress.sin_addr.s_addr = inet_addr(sock.addr);
	clientAddress.sin_port = htons((u_short)sock.port);

	SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

	if (clientSocket == INVALID_SOCKET) {
		printf("Creating client socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	iResult = bind(clientSocket, (LPSOCKADDR)&clientAddress, sizeof(clientAddress));

	if (iResult == SOCKET_ERROR)
	{
		printf("Socket bind failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	/** INIT Client **/

	/** INT Server**/

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(sock.addr);
	serverAddress.sin_port = htons((u_short)sock.port);

	/** INT Server**/

	/** CONNECT **/

	//Slanje "Connected" serveru

	int clientBuffer = CONNECTED;

	iResult = sendto(clientSocket,
		(char *)clientBuffer,
		sizeof(int),
		0,
		(LPSOCKADDR)&serverAddress,
		sockAddrLen);

	if (iResult == SOCKET_ERROR) {
		printf("Sendto failed with error: %d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	printf("Poslat zahtev za komunikaciju\n");

	//RecvFrom Server - ocekuje se Accepted

	iResult = recvfrom(clientSocket,
		(char *)clientBuffer,
		sizeof(int),
		0,
		(LPSOCKADDR)&serverAddress,
		&sockAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	if (clientBuffer == ACCEPTED) {

		printf("Connected to server");
		h->state = CONNECTED;
	}

	/** CONNECT **/

	// while start, (slider < datalen)
	while (h->slider < len)
	{
		/** SEND **/
		// dejina funkcija SEND
		iResult = sendto(
			clientSocket,
			data + h->slider,
			h->cwnd,
			0,
			(LPSOCKADDR)&serverAddress,
			sizeof(struct sockaddr));

		if (iResult == SOCKET_ERROR)
		{
			printf("sendto failed with error: %d\n", WSAGetLastError());
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}

		/** RECV **/
		// dejina funkcija RECV
		//recv = UDPRecv(**);

		/** ALGORITAM **/
		// Ako je u slow start modu, uvecavaj eksponencionalno
		if (h->slowstart)
		{
			// Ako je primljeno manje nego sto je poslato, preci u Tahoe mod i ponovo poslati iste pakete
			if (h->cwnd < h->recv)
			{
				h->ssthresh = h->cwnd / 2;
				h->cwnd = h->ssthresh;
				h->slowstart = false;
				continue;
			}
			// Ako nema problema, povecati cwnd 2x
			else
			{
				h->cwnd = h->cwnd * 2;
			}
		}
		// Ako je u Tahoe modu, povecavati cwnd za 1 u svakoj iteraciji
		else
		{
			// Ako je primljeno manje nego sto je poslato, postaviti cwnd na ssthresh i poslati ponovo
			if (h->cwnd < h->recv)
			{
				h->cwnd = h->ssthresh;
				continue;
			}
			// Ako nema problema, povecati cwnd za 1
			else
			{
				h->cwnd = h->cwnd + 1;
			}
		}

		/** SLIDER **/
		h->slider = h->slider + h->recv;
     
	}
	// while end

	free(h);

	return 0;
}

