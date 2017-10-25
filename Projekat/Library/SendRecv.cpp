#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define MAX_BUFFER_SIZE 10*1024*1024

#include "header.h"

DWORD WINAPI FromDataToBuffer(LPVOID param);
DWORD WINAPI SendDataFromBuffer(LPVOID param);

DWORD WINAPI EMPTYBUFFER(LPVOID param)
{
	rHelper *h;
	h = (rHelper*)param;
	char* empty;
	empty = (char*)malloc(100);

	WaitForSingleObject(h->lock, INFINITE);
	while (h->slider != h->length)
	{
		rPop(&(h->buffer), empty, 100);

		printf("pop 100\n");

		ReleaseSemaphore(h->lock, 1, NULL);
		Sleep(2000);
		WaitForSingleObject(h->lock, INFINITE);
	}
	ReleaseSemaphore(h->lock, 1, NULL);
	return 0;
}

int Send(rSocket sock, char* data, int len)
{

	/** INICIJALIZACIJA **/
	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));
	h->length = len;
	h->data = data;
	h->slider = 0;
	h->state = DISCONNECTED;

	h->cwnd = 10;			// Inicijalno 10
	h->ssthresh = 0;
	h->recv = 1000;			// Inicijalno mora biti veci od cwnd
	h->slowstart = true;

	//Kruzni_Buffer buffer;
	if (len < MAX_BUFFER_SIZE)
	{
		rInitBuffer(&(h->buffer), len);
		rPush(&(h->buffer), data, len);
	}
	else
	{
		rInitBuffer(&(h->buffer), MAX_BUFFER_SIZE);
		rPush(&(h->buffer), data, MAX_BUFFER_SIZE);
	}

	h->lock = CreateSemaphore(0, 1, 1, NULL);
	/** INICIJALIZACIJA **/

	int iResult = 0;

	/** INIT Client **/
	SOCKET clientSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

	if (clientSocket == INVALID_SOCKET) {
		printf("Creating client socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	int sockAddrLen = sizeof(struct sockaddr);

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(sock.addr);
	serverAddress.sin_port = htons((u_short)sock.port);

	h->socket = &clientSocket;
	h->adresa = &serverAddress;

	/** INIT Client **/

	/** CONNECT **/
	int message[2];
	message[0] = REQUEST;
	message[1] = len;

	iResult = sendto(clientSocket,
		(char*)&message,
		2*sizeof(int),
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
		(char*)&message,
		sizeof(int),
		0,
		(LPSOCKADDR)&serverAddress,
		&sockAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	if (message[0] == ACCEPTED) {

		printf("Connected to server");
		h->state = CONNECTED;
	}
	else if (message[0] == REJECTED)
	{
		printf("Server rejected connection");
		h->state = DISCONNECTED;
	}
	/** CONNECT **/


	// HOW TO SEND
	/*char* tempbuffer;
	tempbuffer = (char*)malloc(64 * 1024);

	rMessageHeader* header;
	header = (rMessageHeader*)tempbuffer;
	header->id = 1;
	header->size = h->cwnd;

	rRead(&(h->buffer), tempbuffer + sizeof(rMessageHeader), h->cwnd);

	iResult = sendto(
	*(h->socket),
	tempbuffer,
	h->cwnd + sizeof(rMessageHeader),
	0,
	(LPSOCKADDR)h->adresa,
	sizeof(struct sockaddr));
*/

	getchar();



	//thread1 petlja koja uzima iz data i stavlja u buffer
	HANDLE thread1 = CreateThread(NULL, 0, &FromDataToBuffer, h, 0, NULL);
	//HANDLE thread3 = CreateThread(NULL, 0, &EMPTYBUFFER, h, 0, NULL);

	//thread2 petlja koja uzima iz buffer i salje preko mreze
	HANDLE thread2 = CreateThread(NULL, 0, &SendDataFromBuffer, h, 0, NULL);

	getchar();


	//t2


	// while start, (slider < datalen)
	while (h->slider < len)
	{

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

	rFreeBuffer(&(h->buffer));
	free(h);

	return 0;
}

DWORD WINAPI FromDataToBuffer(LPVOID param)
{
	rHelper* h = (rHelper*)param;
	int s = 0;
	// lock h
	WaitForSingleObject(h->lock, INFINITE);
	while (h->length - h->slider != 0)
	{
		if (h->buffer.free > 0)
		{
			if ((h->length - h->slider) < h->buffer.free)
			{
				s = rPush(&(h->buffer), h->data + h->slider, h->length - h->slider);
				if (s == -1)
					return -1;
				h->slider = h->length;
			}
			else
			{
				s = rPush(&(h->buffer), h->data + h->slider, h->buffer.free);
				if (s == -1)
					return -1;
				h->slider += s;
			}
			printf("\nPUSH");
		}
		//unlock h
		ReleaseSemaphore(h->lock, 1, NULL);
		Sleep(100);
		//lock h
		WaitForSingleObject(h->lock, INFINITE);
	}
	//unlock h
	ReleaseSemaphore(h->lock, 1, NULL);

	return 0;
}

DWORD WINAPI SendDataFromBuffer(LPVOID param)
{
	rHelper* h = (rHelper*)param;
	int iResult;
	int id = 0;
	char* tempbuffer;
	tempbuffer = (char*)malloc(64 * 1024);

	rMessageHeader* header;
	header = (rMessageHeader*)tempbuffer;

	//ovde cemo primiti poruku
	
	int sockAddrLen = sizeof(struct sockaddr);

	while (h->length - h->slider != 0 || h->buffer.taken > 0)
	{
		header->id = ++id;
		header->size = h->cwnd;

		rRead(&(h->buffer), tempbuffer + sizeof(rMessageHeader), h->cwnd);

		iResult = sendto(
			*(h->socket),
			tempbuffer,
			h->cwnd + sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)h->adresa,
			sizeof(struct sockaddr));

		printf("\nSENT %d", header->size);

		iResult = recvfrom(*(h->socket),
			tempbuffer,
			sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)&(*(h->adresa)),
			&sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			return 1;
		}

		//ako je okej stigla poruka
		if (header->state == RECIEVED) {

			rDelete(&(h->buffer), header->size);
		}
		else {
			continue;
		}

		
		// unlock buffer
		// unlock h

		//recvfrom();
		// lock h
		// lock buffer

		//ako je primio dobro rDelete
		//ako ima greska salji ponovo

		// unlock buffer
		// unlock h
		Sleep(300);
		// lock h
		// lock buffer
	}
	// unlock buffer
	// unlock h

	return 0;
}



