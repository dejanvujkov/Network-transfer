#include "Initialize.h"

void Inicijalizuj(rHelper *h, rSocket *sock, char* data, int len) {

	
	h->length = len;
	h->data = data;
	h->slider = 0;
	h->state = DISCONNECTED;

	h->cwnd = 10;			// Inicijalno 10
	h->ssthresh = 0;
	h->recv = 0;			// Inicijalno mora biti veci od cwnd
	h->slowstart = true;

	h->sockAddrLen = sizeof(struct sockaddr);
	
	sockaddr_in* serverAddress;
	serverAddress = (sockaddr_in*)malloc(sizeof(sockaddr_in)); /*CURENJE MEMORIJE*/

	memset((char*)serverAddress, 0, sizeof(sockaddr_in));
	serverAddress->sin_family = AF_INET;
	serverAddress->sin_addr.s_addr = inet_addr(sock->addr);
	serverAddress->sin_port = htons((u_short)sock->port);

	SOCKET *clientSocket;
	clientSocket = (SOCKET*)malloc(sizeof(SOCKET)); /*CURENJE MEMORIJE*/

	*clientSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

	if (*clientSocket == INVALID_SOCKET) {
		printf("Creating client socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return;
	}

	h->socket = clientSocket;
	h->adresa = serverAddress;

	if (len < MAX_BUFFER_SIZE)
	{
		rInitBuffer(&(h->buffer), len);
		rPush(&(h->buffer), data, len);
		h->slider = len;
	}
	else
	{
		rInitBuffer(&(h->buffer), MAX_BUFFER_SIZE);
		rPush(&(h->buffer), data, MAX_BUFFER_SIZE);
		h->slider = MAX_BUFFER_SIZE;
	}

	h->lock = CreateSemaphore(0, 1, 1, NULL);
}

void Initialize(rHelper* h, SOCKET* socket, char* buffer, int size, sockaddr* adresa, int tolen)
{
	(*h).length = size;
	(*h).data = buffer;
	(*h).slider = 0;
	(*h).state = DISCONNECTED;

	(*h).cwnd = 10;
	(*h).ssthresh = 0;
	(*h).recv = 0;//(*h).cwnd + 1;
	(*h).slowstart = true;

	(*h).sockAddrLen = tolen;
	(*h).socket = socket;
	(*h).adresa = (sockaddr_in*)adresa;

	int bfSize;
	if (size < MAX_BUFFER_SIZE)
		bfSize = size;
	else
		bfSize = MAX_BUFFER_SIZE;

	rInitBuffer(&(h->buffer), bfSize);
	rPush(&(h->buffer), buffer, bfSize);
	h->slider = bfSize;

	h->lock = CreateSemaphore(0, 1, 1, NULL);
}