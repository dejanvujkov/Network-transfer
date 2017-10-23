#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "header.h"
#include <stdlib.h>
#include <stdio.h>

int Send(rSocket sock, char* data, int len)
{
	/** INICIJALIZACIJA **/
	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));
	h->slider = 0;
	h->state = DISCONECTED;

	h->cwnd = 10;			// Inicijalno 10
	h->ssthresh = 0;
	h->recv = 1000;			// Inicijalno mora biti veci od cwnd
	h->slowstart = true;
	


	int sockAddrLen = sizeof(struct sockaddr);

	// INIT Client
	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(sock.addr);
	serverAddress.sin_port = htons((u_short)sock.port);

	SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP
	// INIT Client

	// INIT Server
	sockaddr_in localServerAddress;
	memset((char*)&localServerAddress, 0, sizeof(localServerAddress));
	localServerAddress.sin_family = AF_INET; /*set server address protocol family*/
	localServerAddress.sin_addr.s_addr = INADDR_ANY;
	localServerAddress.sin_port = htons(sock.port + 1);

	SOCKET serverSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP
	
	bind(serverSocket, (LPSOCKADDR)&localServerAddress, sizeof(localServerAddress));
	// INIT Server
	

	int iResult = 0;

	for (int i = 0; i < 4; i++)
		printf("  %x  ", ((char*)&len)[i]);
	
	/** CONNECT **/
	iResult = sendto(
		clientSocket,
		(char*)&len,
		4,
		0,
		(LPSOCKADDR)&serverAddress,
		sockAddrLen);



	char* buffer;
	buffer = (char*)malloc(100);

	iResult = recvfrom(serverSocket,
		buffer,
		4,
		0,
		(LPSOCKADDR)&localServerAddress,
		&sockAddrLen);

	for (int i = 0; i < 4; i++)
		printf("  %x  ", buffer[i]);


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

