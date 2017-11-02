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

	int brojPaketa;
	int velicinaPoruke = MAX_UDP_SIZE - sizeof(rMessageHeader);
	int trenutnoProcitano;
	int procitano;

	int i;

	while (s->activeThreads)
	{
		if (s->sendBuffer->taken > 0 && s->state == CONNECTED && s->canSend)
		{
			brojPaketa = (s->cwnd) / velicinaPoruke;
			procitano = 0;
			printf("\nMOGU DA SALJEM %d PAKETA", brojPaketa + 1);
			s->canSend = false;
			s->timedOut = false;
			s->brojPoslednjePoslatih = brojPaketa + 1;

			for (i = 0; i <= brojPaketa; i++)
			{
				header->type = DATA;
				header->id = ++s->idPoslednjePoslato;
				header->size = (brojPaketa != i && brojPaketa != 0) ? velicinaPoruke : (s->cwnd % velicinaPoruke);
				
				trenutnoProcitano = rRead(s->sendBuffer, data, header->size);

				if (trenutnoProcitano > s->sendBuffer->taken - procitano)
				{
					header->size = s->sendBuffer->taken - procitano;
					brojPaketa = i;
					s->brojPoslednjePoslatih = i + 1;
				}
				
				procitano += trenutnoProcitano;

				iResult = sendto(s->socket, tempBuffer, sizeof(rMessageHeader) + header->size, 0, (LPSOCKADDR)(s->adresa), sizeof(sockaddr_in));
				
				if (iResult == SOCKET_ERROR)
				{
					printf("\n SEND failed with error: %d\n", WSAGetLastError());
					continue;
				}

				printf("\n SEND: ID:[%d] B:[%d] ", header->id, header->size);
			}

		}
		else
			Sleep(10);
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

	char* ackBuffer = (char*)malloc(2000 * sizeof(rMessageHeader));
	int ackCount = 0;

	while (s->activeThreads)
	{
		if (s->adresa->sin_port == ntohs(0))
			continue;

		iResult = recvfrom(s->socket, tempBuffer, MAX_UDP_SIZE, 0, (LPSOCKADDR)s->adresa, &(s->sockAddrLen));
		
		// Ako je time out
		if (iResult == SOCKET_ERROR)
		{
			iResult = WSAGetLastError();
			if (iResult != 10060)
			{
				//printf("\nrecvfrom failed with error: %d\n", iResult);
				continue;
			}
			else
			{
				printf("\nTIME OUT");
				//if (s->brojPoslednjePrimljenih < s->brojPoslednjePoslatih)
				if (ackCount > 0)
				{
					s->canSend = false;
					s->timedOut = true;
					CountACKs(s, ackBuffer, ackCount);
					Algoritam(s);
					s->recv = 0;
					ackCount = 0;
					s->canSend = true;
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
				//printf("recvfrom failed with error: %d\n", WSAGetLastError());
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

			printf("\n [%d] Recieved B: %d ", header->id, header->size);

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
			
			if (s->timedOut)
				continue;

			memcpy(ackBuffer + ackCount * sizeof(rMessageHeader), header, sizeof(rMessageHeader));
			ackCount++;

			printf("\nACK [%d]", header->id);

			// Ako je primio sve ocekivano ACK, odradi algoritam i daje dozvolu za slanje
			if (ackCount == s->brojPoslednjePoslatih)
			{
				s->canSend = false;
				CountACKs(s, ackBuffer, ackCount);
				Algoritam(s);
				ackCount = 0;
				s->recv = 0;
				s->canSend = true;
			}
			break;
		}
		default:
			break;
		}
	}

	free(tempBuffer);
	free(ackBuffer);

	return 0;
}

//Obrada ACK
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

	s->idPoslednjePoslato = s->idOcekivanog - 1;
}

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
			if (h->cwnd <= h->ssthresh + h->ssthresh / 10)
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