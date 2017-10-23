#include "header.h"
#include "Algoritam.h"
#include <stdlib.h>

int Send(char* data, int len, SOCKET* socket, LPSOCKADDR* serveraddress, int addrLen)
{
	/** INICIJALIZACIJA **/
	SendRecvHelper* h;
	h = (SendRecvHelper*)malloc(sizeof(SendRecvHelper));

	h->data = data;
	h->datalen = len;
	h->slider = 0;

	h->clientsocket = socket;
	h->address = serveraddress;
	h->addresslength = addrLen;

	h->cwnd = 10;			// Inicijalno 10
	h->ssthresh = 0;
	h->recv = 1000;			// Inicijalno mora biti veci od cwnd
	h->slowstart = true;

	// while start, (slider < datalen)
	while (h->slider < h->datalen)
	{
		/** SEND **/
		// dejina funkcija SEND
		//UDPSend(h->data + h->slider, h->cwnd, h->clientsocket, h->address, h->addresslength);


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
	//UDPCleanup();

	return 0;
}

