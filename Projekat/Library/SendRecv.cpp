#include "header.h"

rSocket* rInitialize()
{
	int iResult = 0;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return NULL;
	}
	
	rSocket* s;
	s = (rSocket*)malloc(sizeof(rSocket));
	if (s == NULL)
		return NULL;

	s->socket = socket(AF_INET,
		SOCK_DGRAM,
		IPPROTO_UDP);

	s->adresa = (sockaddr_in*)calloc(0, sizeof(sockaddr_in));
	if (s->adresa == NULL)
		return NULL;

	s->cwnd = INITIAL_CWND;
	s->ssthresh = 0;
	s->recv = 0;
	s->slowstart = true;

	s->state = DISCONNECTED;
	s->sockAddrLen = sizeof(sockaddr_in);

	s->sendBuffer = (Kruzni_Buffer*)malloc(2 * sizeof(Kruzni_Buffer));
	rInitBuffer(s->sendBuffer, MAX_BUFFER_SIZE);
	if (s->sendBuffer->buffer_start == NULL)
		return NULL;
	s->recvBuffer = s->sendBuffer + 1;
	rInitBuffer(s->recvBuffer, MAX_BUFFER_SIZE);
	if (s->recvBuffer->buffer_start == NULL)
		return NULL;

	s->activeThreads = true;
	s->sendThread = (HANDLE*)malloc(2* sizeof(HANDLE));
	s->recvThread = s->sendThread + 1;
	*(s->sendThread) = CreateThread(NULL, 0, &SendThread, s, 0, NULL);
	*(s->recvThread) = CreateThread(NULL, 0, &RecvThread, s, 0, NULL);
	
	s->brojPoslednjePoslatih = 0;
	s->idPoslednjePoslato = 0;
	s->idOcekivanog = 1;
	s->timedOut = false;
	s->canSend = true;

	return s;
}

int rDeinitialize(rSocket* s)
{	
	rFreeBuffer(s->sendBuffer);
	rFreeBuffer(s->recvBuffer);

	free(s->sendBuffer);
	free(s->sendThread);

	free(s->adresa);
	free(s);

	// Gasi threadove
	s->activeThreads = false;

	return 0;
}

int rConnect(rSocket* s, char* serverAddress, short port)
{
	int iResult;
	//connect
	(s->adresa)->sin_family = AF_INET;
	(s->adresa)->sin_addr.s_addr = inet_addr(serverAddress);
	(s->adresa)->sin_port = htons((u_short)port);

	DWORD timeout = TIMEOUT_SEC;

	setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	rMessageHeader header;
	header.type = REQUEST;
	header.id = 0;
	header.size = 0;

	iResult = sendto(s->socket, (char*)&header, sizeof(rMessageHeader), 0, (LPSOCKADDR)s->adresa, s->sockAddrLen);
	if (iResult == SOCKET_ERROR)
	{
		printf("//recvfrom failed with error: %d\n", WSAGetLastError());
		return -1;
	}

	return 0;
}

int rAccept(rSocket* s, short port)
{
	DWORD timeout = TIMEOUT_SEC;

	s->adresa->sin_family = AF_INET;
	s->adresa->sin_addr.s_addr = INADDR_ANY;
	s->adresa->sin_port = htons((u_short)port);

	bind(s->socket, (LPSOCKADDR)s->adresa, s->sockAddrLen);

	setsockopt(s->socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	return 0;
}

int rSend(rSocket* s, char* data, int len)
{
	int iResult;

	// Ako nije uspostavljena konekcija, uspostavlja se ovde
	while (s->state == DISCONNECTED)
	{
		Sleep(100);
		continue;
	}

	while (s->sendBuffer->free < len)
	{
		iResult = rResize(s->sendBuffer, (s->sendBuffer->buffer_end - s->sendBuffer->buffer_start) * 2);
	}

	iResult = rPush(s->sendBuffer, data, len);

	if (iResult != len)
	{
		printf("nije pushovano na buffer");
		return -1;
	}

	return 0;
}

int rRecv(rSocket* s, char* data, int len)
{
	int iResult;

	do {
		iResult = rRead(s->recvBuffer, data, len);
		Sleep(100);
	} while (iResult != len);

	printf("SVE SA BUFFERA %d", iResult);

	return iResult;
}