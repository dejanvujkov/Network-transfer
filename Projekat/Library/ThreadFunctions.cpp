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
	tempbuffer = (char*)malloc(64 * 1024);

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
			rDelete(&(h->buffer), header->size);
			//ALGORITAM
		}
		// Ako nije dostavljena ponovo se salje, bez pomeranja buffera
		else 
		{
			continue;
		}

		ReleaseSemaphore(h->lock, 1, NULL);
		// ALGORITAM OVDE
		WaitForSingleObject(h->lock, INFINITE);
	}
	ReleaseSemaphore(h->lock, 1, NULL);

	free(tempbuffer);

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

