#include "header.h"

DWORD WINAPI SendThread(LPVOID param)
{
	int iResult;

	rSocket* s = (rSocket*)param;

	// Buffer za UDP poruku
	char* tempBuffer = (char*)malloc(MAX_UDP_SIZE);
	// Header i data pokazivaci u bufferu
	rMessageHeader* header = (rMessageHeader*)tempBuffer;
	char* data = tempBuffer + sizeof(rMessageHeader);

	// Za oznacavanje paketa
	int idPoslednjePoslato = 0;

	int brojPaketa;
	int velicinaPoruke = MAX_UDP_SIZE - sizeof(rMessageHeader);
	int trenutnoProcitano;
	int procitano;

	int i;

	while (s->activeThreads)
	{
		if (s->sendBuffer->taken > 0 && s->state == CONNECTED && s->canSend)
		{
			brojPaketa = (s->cwnd) / (s->sendBuffer->taken);
			procitano = 0;
			printf("\nMOGU DA SALJEM %d PAKETA", brojPaketa + 1);

			for (i = 0; i <= brojPaketa; i++)
			{
				header->type = DATA;
				header->id = ++idPoslednjePoslato;
				header->size = (brojPaketa != i && brojPaketa != 0) ? velicinaPoruke : (s->cwnd % velicinaPoruke);
				
				trenutnoProcitano = rRead(s->sendBuffer, data, header->size);

				if (trenutnoProcitano > s->sendBuffer->taken - procitano)
				{
					header->size = s->sendBuffer->taken - procitano;
					brojPaketa = i;
				}
				
				procitano += trenutnoProcitano;

				iResult = sendto(s->socket, tempBuffer, sizeof(rMessageHeader) + header->size, 0, (LPSOCKADDR)s->adresa, sizeof(s->sockAddrLen));
				
				if (iResult == SOCKET_ERROR)
				{
					printf("\n SEND failed with error: %d\n", WSAGetLastError());
					continue;
				}

				printf("\n SEND: ID:[%d] B:[%d] ", header->id, header->size);
			}

			printf("\nZAVRSIO SLANJE CEKA ACK");

			s->canSend = false;
		}
		else
			Sleep(1000);
	}

	free(tempBuffer);

	return 0;
}
DWORD WINAPI RecvThread(LPVOID param)
{
	int iResult;

	rSocket* s = (rSocket*)param;

	// Buffer za UDP poruku
	char* tempBuffer = (char*)malloc(MAX_UDP_SIZE);
	// Pokazivaci na podatke
	rMessageHeader* header = (rMessageHeader*)tempBuffer;
	char* data = tempBuffer + sizeof(rMessageHeader);

	char* ackBuffer = (char*)malloc(100 * sizeof(rMessageHeader));
	int ackCount = 0;

	while (s->activeThreads)
	{
		iResult = recvfrom(s->socket, (char*)header, sizeof(rMessageHeader), 0, (LPSOCKADDR)s->adresa, &(s->sockAddrLen));
		
		// Ako je time out
		if (iResult == SOCKET_ERROR)
		{
			iResult = WSAGetLastError();
			if (iResult != 10060)
			{
				printf("\nrecvfrom failed with error: %d\n", iResult);
				continue;
			}
			else
			{
				printf("\nTIME OUT");
				if (s->brojPoslednjePrimljenih < s->brojPoslednjePoslatih)
				{
					CountACKs(s, ackBuffer, ackCount);
					Algoritam(s);
					s->recv = 0;
					ackCount = 0;
					s->canSend = true;
					s->timedOut = false;
				}
				continue;
			}
		}

		// Ako je primljena poruka gleda se tip poruke
		switch (header->type)
		{
		case REQUEST:
		{
			if (s->state == DISCONNECTED)
			{
				header->type = ACCEPT;
				header->id = 0;
				header->size = 0;

				s->state = CONNECTED;
			}
			else
			{
				header->type = REJECT;
				header->id = 0;
				header->size = 0;
			}

			iResult = sendto(s->socket, (char*)header, sizeof(rMessageHeader), 0, (LPSOCKADDR)s->adresa, s->sockAddrLen);

			if (iResult == SOCKET_ERROR)
			{
				printf("recvfrom failed with error: %d\n", WSAGetLastError());
				s->state = DISCONNECTED;
				continue;
			}
			break;
		}
		case ACCEPT:
		{
			s->state = CONNECTED;
			break;
		}
		case REJECT:
		{
			s->state = DISCONNECTED;
			break;
		}
		case DATA:
		{
			if (s->state == DISCONNECTED)
				continue;
			// Ako je podatak prima se ostatak poruke
			iResult = recvfrom(s->socket, data, header->size, 0, (LPSOCKADDR)s->adresa, &(s->sockAddrLen));
			if (iResult == SOCKET_ERROR)
			{
				printf("DATA DATA recvfrom failed with error: %d\n", WSAGetLastError());
				continue;
			}

			while (s->recvBuffer->free < header->size)
			{
				iResult = rResize(s->recvBuffer, (s->recvBuffer->buffer_end - s->recvBuffer->buffer_start) * 2);
				if (iResult != 0)
				{
					printf("\nBUFFER NE MOZE DA SE UVECA");
					continue;
				}
			}

			rPush(s->recvBuffer, data, header->size);

			header->type = ACK;

			iResult = sendto(s->socket, (char*)header, sizeof(rMessageHeader), 0, (LPSOCKADDR)s->adresa, s->sockAddrLen);
			if (iResult == SOCKET_ERROR)
			{
				printf("DATA ACK-> recvfrom failed with error: %d\n", WSAGetLastError());
				continue;
			}
			break;
		}
		case ACK:
		{
			if (s->state == DISCONNECTED)
				continue;
			//Ako nije timeout obradi ack
			if (!s->timedOut)
			{
				ackCount++;	//mora biti 0 negde
				memcpy(ackBuffer + ackCount * sizeof(rMessageHeader), header, sizeof(rMessageHeader));

				// Ako je primio sve ocekivano ACK, odradi algoritam i daje dozvolu za slanje
				if (ackCount == s->brojPoslednjePrimljenih)
				{
					CountACKs(s, ackBuffer, ackCount);
					Algoritam(s);
					ackCount = 0;
					s->recv = 0;
					s->canSend = true;
					s->timedOut = false;
				}
				break;

			}
			else
			{
				continue;
				// ACK zakasnio, ne radi se nista
			}
			break;
		}
		default:
			break;
		}

		//zaglavi u recv
		//kada primi vidi sta ces
	}

	free(tempBuffer);
	free(ackBuffer);

	return 0;
}

void CountACKs(rSocket* s, char* ackBuffer, int ackCount)
{
	rMessageHeader* header;

	for (int i = 0; i < ackCount; i++)
	{
		for (int j = 0; j < ackCount + 1; j++)
		{
			header = (rMessageHeader*)(ackBuffer + j * sizeof(rMessageHeader));
			if (j == ackCount + 1)
				return;

			if (header->id == s->idOcekivanog)
			{
				s->recv += header->size;
				rDelete(s->sendBuffer, header->size);
				s->idOcekivanog++;
				break;
			}
		}
	}
}



///////////////

//DWORD WINAPI FromDataToBuffer(LPVOID param)
//{
//	rHelper* h = (rHelper*)param;
//	int s = 0;
//	int tempSize;
//	WaitForSingleObject(h->lock, INFINITE);
//	// Stavlja podatke na buffer dok ne dodje do kraja
//	while (h->length - h->slider != 0)
//	{
//		// Ako buffer ima slobodan prostor, popuni ga
//		if (h->buffer.free > 0)
//		{
//			// velizina ako poruka moze da stane u slobodan prostor
//			if ((h->length - h->slider) < h->buffer.free)
//			{
//				tempSize = h->length - h->slider;
//			}
//			// Velicina ako poruka ne moze da stane u slobodan prostor
//			else
//			{
//				tempSize = h->buffer.free;
//			}
//
//			// Popunjavanje buffera velicinom definisanom iznad
//			s = rPush(&(h->buffer), h->data + h->slider, tempSize);
//			// Pomeranje prozora prebacednog dela poruke
//			h->slider += s;
//
//			printf("\nPUSH %d", s);
//		}
//		ReleaseSemaphore(h->lock, 1, NULL);
//		Sleep(10);
//		WaitForSingleObject(h->lock, INFINITE);
//	}
//	ReleaseSemaphore(h->lock, 1, NULL);
//
//	return 0;
//}
//
//DWORD WINAPI SendDataFromBuffer(LPVOID param)
//{
//	rHelper* h = (rHelper*)param;
//
//	char* tempbuffer;
//	tempbuffer = (char*)malloc(MAX_UDP_SIZE);
//
//	rMessageHeader* header;
//
//	int idPoslednjePoslato = 0;
//	int idPoslednjePrimljeno = 0;
//
//	int brojPaketa;
//	int velicinaPoruke = MAX_UDP_SIZE - sizeof(rMessageHeader);
//
//	int trenutnoProcitano;
//	int procitano;
//
//	// Salje poruke dok ne posalje sve
//	WaitForSingleObject(h->lock, INFINITE);
//	while (h->length - h->slider != 0 || h->buffer.taken > 0)
//	{
//		// Na koliko paketa se rastavlja poruka
//		brojPaketa = (h->cwnd) / velicinaPoruke;
//		h->recv = 0;
//
//		procitano = 0;
//		
//		header = (rMessageHeader*)tempbuffer;
//
//		for (int i = 0; i <= brojPaketa; i++) 
//		{
//			// SLANJE	
//			SendOneMessage(header, &idPoslednjePoslato, &brojPaketa, i, velicinaPoruke, h, &trenutnoProcitano, &procitano, tempbuffer);
//			//Sleep(100); // Delay da server moze da isprati
//		}
//
//		ReleaseSemaphore(h->lock, 1, NULL);
//
//		for (int i = 0; i <= brojPaketa; i++)
//		{
//			// PRIMANJE
//			if (RecvOneMessage(h, tempbuffer, i) == -1)
//			{
//				// Doslo to timeout-a
//				brojPaketa = i;
//				break;
//			}
//		}
//
//		WaitForSingleObject(h->lock, INFINITE);
//
//		for (int i = 0; i <= brojPaketa; i++)
//		{
//
//			for (int j = 0; j <= brojPaketa + 1; j++)
//			{
//				header = (rMessageHeader*)(tempbuffer + j * sizeof(rMessageHeader));
//				// Ako nije nasao ocekivani paket, izlazi iz obe petlje
//				if (j == brojPaketa + 1)
//				{
//					// DOSLO JE DO DROPA
//					idPoslednjePoslato = idPoslednjePrimljeno;
//					break;
//				}
//				// Ako je nasao zeljeni paket, sliduje ga
//				if (header->id == idPoslednjePrimljeno + 1)
//				{
//					SlideOneMessage(header, tempbuffer, j, h); // proveri zakomentarisan header, mozda nepotrebni parametri
//					idPoslednjePrimljeno++;
//					break;
//				}
//			}
//
//
//			// KLIZAJUCI PROZOR		
//			//SlideOneMessage(header, tempbuffer, i, h);
//			// NEPOTREBNO ???
//		}
//
//		Algoritam(h);
//
//		//printf("[%d]\tB:%d\tCWND:%d\t\n", header->id, header->size, h->cwnd);
//
//		Sleep(10);
//	}
//
//	ReleaseSemaphore(h->lock, 1, NULL);
//
//	free(tempbuffer);
//
//	return h->slider;
//}
//
//DWORD WINAPI RecieveMessage(LPVOID param)
//{
//	rClientMessage* clientInfo = (rClientMessage*)param;
//	//WaitForSingleObject(clientInfo->lock, INFINITE);
//
//	int id = 1;
//
//	int iResult;
//
//	int sockAddrLen = sizeof(struct sockaddr);
//
//	// Buffer za svaku poruku
//	char* accessBuffer;
//	accessBuffer = (char*)malloc(MAX_UDP_SIZE);
//	if (accessBuffer == NULL)
//		return 1;
//
//	rMessageHeader* header = (rMessageHeader*)accessBuffer;
//	char* message = accessBuffer + sizeof(rMessageHeader);
//	// NEBITNO memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
//
//	// Prima svaki paket
//	while (clientInfo->messageSize - clientInfo->slider > 0)
//	{
//		iResult = recvfrom(clientInfo->socket, accessBuffer, MAX_UDP_SIZE, 0, (LPSOCKADDR)clientInfo->clientAddress, &sockAddrLen);
//
//		if (iResult == SOCKET_ERROR)
//		{
//			printf("\nrecvfrom failed with error: %d", WSAGetLastError());
//			continue;
//		}
//
//		if (header->state == REQUEST)
//		{
//			// Ako stigne request od drugog klijenta, salje se reject
//			header->state = REJECTED;
//
//			iResult = sendto(clientInfo->socket,
//				accessBuffer,
//				sizeof(rMessageHeader),
//				0,
//				(LPSOCKADDR)clientInfo->clientAddress,
//				sockAddrLen);
//
//			continue;
//		}
//
//		if (header->id != id)
//			header->state = DROPPED;
//		else
//			header->state = RECIEVED;
//
//		iResult = sendto(clientInfo->socket,
//			accessBuffer,
//			sizeof(rMessageHeader),
//			0,
//			(LPSOCKADDR)clientInfo->clientAddress,
//			sockAddrLen);
//
//		if (iResult == SOCKET_ERROR)
//		{
//			printf("\nrecvfrom failed with error: %d", WSAGetLastError());
//			continue;
//		}
//
//		if (header->id != id)
//		{
//			printf("\n[%d] dropped, expected [%d]", header->id, id);
//			continue;
//		}
//
//		id++;
//		printf("\n[%d] Recieved: %d ", header->id, header->size);
//		memcpy(message, clientInfo->buffer, header->size);
//		clientInfo->slider += header->size;
//
//		printf("\t\tTotal: %d", clientInfo->slider);
//	}
//
//	*(clientInfo->lock) = true;
//	//ReleaseSemaphore(*(clientInfo->lock), 1, NULL);
//	free(clientInfo->clientAddress);
//	//free(clientInfo);
//	free(accessBuffer);
//
//	free(clientInfo->buffer);
//
//	return 0;
//}

int Algoritam(rSocket* h)
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
			// TREBA GA JOS MALO SMANJITI
			if (h->cwnd <= h->ssthresh + h->ssthresh/10)
				h->ssthresh /= 2;
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

//int SendOneMessage(rMessageHeader* header, int* idPoslednjePoslato, int* brojPaketa, int i, int velicinaPoruke, rHelper* h, int* trenutnoProcitano, int* procitano, char* tempbuffer)
//{
//	int iResult;
//	//Sleep(10);
//
//	/*if (h->slider == h->length)
//		printf("");*/
//
//	header->id = ++(*idPoslednjePoslato);
//
//	// Koliko bajtova moze da stane u poruku
//	header->size = (*brojPaketa != i && *brojPaketa != 0) ? velicinaPoruke : (h->cwnd % velicinaPoruke);
//
//	// Koliko je zapravo procitano
//	*trenutnoProcitano = rRead(&(h->buffer), tempbuffer + sizeof(rMessageHeader), header->size);
//
//	// Ako je procitano vise nego sto moze
//	if (*trenutnoProcitano > h->buffer.taken - *procitano)
//	{
//		header->size = h->buffer.taken - *procitano;
//		*brojPaketa = i;
//	}
//
//	*procitano += *trenutnoProcitano;
//
//	iResult = sendto(
//		*(h->socket),
//		tempbuffer,
//		header->size + sizeof(rMessageHeader),
//		0,
//		(LPSOCKADDR)h->adresa,
//		sizeof(struct sockaddr));
//
//	if (iResult == SOCKET_ERROR)
//	{
//		printf("recvfrom failed with error: %d\n", WSAGetLastError());
//		return -1;
//	}
//
//	printf("\n[%d] B: %d", header->id, header->size);
//
//	return 0;
//}
//
//int RecvOneMessage(rHelper* h, char* tempbuffer, int i)
//{
//	int iResult;
//	rMessageHeader* header = (rMessageHeader*)(tempbuffer + i * sizeof(rMessageHeader));
//
//	iResult = recvfrom(
//		*(h->socket),
//		tempbuffer + i * sizeof(rMessageHeader),
//		sizeof(rMessageHeader),
//		0,
//		(LPSOCKADDR)&(*(h->adresa)),
//		&h->sockAddrLen
//	);
//
//	if (iResult == SOCKET_ERROR)
//	{
//		iResult = WSAGetLastError();
//		if (iResult == 10060)
//		{
//			//TIMED OUT
//			return -1;
//		}
//		printf("recvfrom failed with error: %d\n", WSAGetLastError());
//		return 1;
//	}
//
//	printf("\n[%d] ACK", header->id);
//
//	return 0;
//}
//
//int SlideOneMessage(rMessageHeader* header, char* tempbuffer, int i, rHelper* h)
//{
//	//header = (rMessageHeader*)(tempbuffer + i * sizeof(rMessageHeader));
//
//	// Ako je poruka dostavljena, brise se iz buffera
//	if (header->state == RECIEVED)
//	{
//		h->recv += header->size;
//		rDelete(&(h->buffer), header->size);
//	}
//
//	return 0;
//}