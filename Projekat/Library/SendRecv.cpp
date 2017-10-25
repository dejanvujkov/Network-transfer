

#include "header.h"

int Send(rSocket sock, char* data, int len)
{

	/** INICIJALIZACIJA **/

	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));

	int sockAddrLen = sizeof(struct sockaddr);
	
	int iResult = 0;

	Inicijalizuj(h, &sock, data, len);

	
	/** CONNECT **/
	rMessageHeader header;
	header.id = REQUEST;
	header.size = len;

	iResult = sendto(*(h->socket),
		(char*)&header,
		sizeof(rMessageHeader),
		0,
		(LPSOCKADDR)(h->adresa),
		sockAddrLen);

	if (iResult == SOCKET_ERROR) {
		printf("Sendto failed with error: %d\n", WSAGetLastError());
		closesocket(*(h->socket));
		WSACleanup();
		return 1;
	}

	printf("Poslat zahtev za komunikaciju\n");


	//RecvFrom Server - ocekuje se Accepted
	iResult = recvfrom(*(h->socket),
		(char*)&header,
		sizeof(rMessageHeader),
		0,
		(LPSOCKADDR)(h->adresa),
		&sockAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	if (header.id == ACCEPTED) {

		printf("Connected to server");
		h->state = CONNECTED;
	}
	else if (header.id == REJECTED)
	{
		printf("Server rejected connection");
		h->state = DISCONNECTED;
	}
	/** CONNECT **/


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




