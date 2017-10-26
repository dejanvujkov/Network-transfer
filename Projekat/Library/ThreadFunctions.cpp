#include "header.h"

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
				if (h->slider > 99000000)
					printf("");

				s = rPush(&(h->buffer), h->data + h->slider, h->buffer.free);
				if (s == -1)
					return -1;
				h->slider += s;
			}
			printf("\nPUSH %d", s);
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

		printf("\n[%d] Sent %d ", header->id, header->size);

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
			//ALGORITAM
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
		//Sleep(10);
		// lock h
		// lock buffer
	}
	// unlock buffer
	// unlock h

	return 0;
}

int Algoritam(rHelper* h)
{
	// Ako je u slow start modu, uvecavaj eksponencionalno
	if (h->slowstart)
	{
		// Ako je primljeno manje nego sto je poslato, preci u Tahoe mod i ponovo poslati iste pakete
		if (h->cwnd < h->recv)
		{
			h->ssthresh = h->cwnd / 2;
			h->cwnd = h->ssthresh;
			h->slowstart = false;
			//continue;
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
			//continue;
		}
		// Ako nema problema, povecati cwnd za 1
		else
		{
			h->cwnd = h->cwnd + 1;
		}
	}

	/** SLIDER **/
	h->slider = h->slider + h->recv;



	return 0;
}

