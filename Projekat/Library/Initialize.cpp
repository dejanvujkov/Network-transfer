#include "header.h"

//void Initialize(rHelper* h, SOCKET* socket, char* buffer, int size, sockaddr* adresa, int tolen)
//{
//	(*h).length = size;
//	(*h).data = buffer;
//	(*h).slider = 0;
//	(*h).state = DISCONNECTED;
//
//	(*h).cwnd = 10;
//	(*h).ssthresh = 0;
//	(*h).recv = 0;//(*h).cwnd + 1;
//	(*h).slowstart = true;
//
//	(*h).sockAddrLen = tolen;
//	(*h).socket = socket;
//	(*h).adresa = (sockaddr_in*)adresa;
//
//	DWORD timeout = WAIT_FOR_ACK_SEC * 1000;
//	setsockopt(*(h->socket), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
//
//	int bfSize;
//	if (size < MAX_BUFFER_SIZE)
//		bfSize = size;
//	else
//		bfSize = MAX_BUFFER_SIZE;
//
//	rInitBuffer(&(h->buffer), bfSize);
//	rPush(&(h->buffer), buffer, bfSize);
//	h->slider = bfSize;
//
//	h->lock = CreateSemaphore(0, 1, 1, NULL);
//}