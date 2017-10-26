#include "header.h"

DWORD WINAPI FromDataToBuffer(LPVOID param)
{
	rHelper* h = (rHelper*)param;
	int s = 0;
	int tempSize;
	WaitForSingleObject(h->lock, INFINITE);
	// Stavlja podatke na buffer dok ne dodje do kraja
	while (h->length - h->slider != 0)
	{
		// Ako buffer ima slobodan prostor, popuni ga
		if (h->buffer.free > 0)
		{
			// velizina ako poruka moze da stane u slobodan prostor
			if ((h->length - h->slider) < h->buffer.free)
			{
				tempSize = h->length - h->slider;
			}
			// Velicina ako poruka ne moze da stane u slobodan prostor
			else
			{
				tempSize = h->buffer.free;
			}

			// Popunjavanje buffera velicinom definisanom iznad
			s = rPush(&(h->buffer), h->data + h->slider, tempSize);
			// Pomeranje prozora prebacednog dela poruke
			h->slider += s;

			printf("\nPUSH %d", s);
		}
		ReleaseSemaphore(h->lock, 1, NULL);
		Sleep(100);
		WaitForSingleObject(h->lock, INFINITE);
	}
	ReleaseSemaphore(h->lock, 1, NULL);

	return 0;
}

DWORD WINAPI SendDataFromBuffer(LPVOID param)
{
	rHelper* h = (rHelper*)param;
	int iResult;
	int id = 0;
	char* tempbuffer;
	tempbuffer = (char*)malloc(MAX_UDP_SIZE);

	rMessageHeader* header;
	header = (rMessageHeader*)tempbuffer;

	//ovde cemo primiti poruku

	int sockAddrLen = sizeof(struct sockaddr);

	WaitForSingleObject(h->lock, INFINITE);
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

		// Dok ne dodje odgovor moze da se oslobodi h
		ReleaseSemaphore(h->lock, 1, NULL);

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

		WaitForSingleObject(h->lock, INFINITE);

		// Ako je poruka dostavljena, brise se iz buffera
		if (header->state == RECIEVED) 
		{
			h->recv += header->size;
			rDelete(&(h->buffer), header->size);
			Algoritam(h);
		}
		// Ako nije dostavljena ponovo se salje, bez pomeranja buffera
		else 
		{
			continue;
		}
	}

	ReleaseSemaphore(h->lock, 1, NULL);

	free(tempbuffer);

	return h->slider;
}

int Algoritam(rHelper* h)
{
	// Ako je primljeno manje nego poslato
	if (h->recv < h->cwnd)
	{
		// Ako je u slowstart modu
		if (h->slowstart)
		{
			// postavlja se ssthresh i cwnd = ssthresh
			h->ssthresh = h->cwnd / 2;
			h->cwnd = h->ssthresh;
			h->slowstart = false;
		}
		// Ako je u Tahoe modu
		else
		{
			// swnd se vraca na ssthresh
			h->cwnd = h->ssthresh;
		}
	}
	// Ako je primljeno isto kao poslato
	else
	{
		// Slowstart -> cwnd * 2
		if (h->slowstart)
			h->cwnd *= 2;
		// Tahoe -> cwnd + 1
		else
			h->cwnd += 1;
	}
	
	return 0;
}

