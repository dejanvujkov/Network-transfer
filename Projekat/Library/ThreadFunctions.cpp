#include "header.h"

DWORD WINAPI SendThread(LPVOID param)
{
	/*  */
	rSocket* s = (rSocket*)param;

	/* Flag greske */
	int iResult;

	/* Buffer za UDP poruku */
	char* tempBuffer = (char*)malloc(MAX_UDP_SIZE);

	/* Pokazivac na HEADER deo poruke */
	rMessageHeader* header = (rMessageHeader*)tempBuffer;
	/* Pokazivac na DATA deo poruke */
	char* data = tempBuffer + sizeof(rMessageHeader);

	/* Pomocni podaci za upravljanje paketima */
	int brojPaketa;												/* Broj poruka koje se salju u jednoj iteraciji */
	int velicinaPoruke = MAX_UDP_SIZE - sizeof(rMessageHeader);	/* Velicina poruke koja se salje */
	int trenutnoProcitano;										/* Duzina podataka procitanih sa buffera */
	int procitano;												/* Duzina poslatih podataka u jednoj iteraciji */

	int i;

	WaitForSingleObject(s->lock, INFINITE);
	/* Thread ostaje aktivan dokle god je rSocket::activeThreads = true */
	while (s->activeThreads)
	{
		/** 
		 * Uslov za slanje poruka
		 * ? Da li ima podataka za slanje na bufferu ?
		 * ? Da li je konekcija uspesno uspostavljena ?
		 * ? Da li su stigli svi ACK-ovi ?
		 */
		if (s->sendBuffer->taken > 0 && s->state == CONNECTED && s->canSend)
		{
			brojPaketa = (s->cwnd) / velicinaPoruke;		/* Racunanje broja poruka u iteraciji */
			printf("\n-- Salje se %d poruka --", brojPaketa + 1);	/* info */
			/* Resetovanje statusnih podataka */
			procitano = 0;
			s->canSend = false;
			s->timedOut = false;
			s->brojPoslednjePoslatih = brojPaketa + 1;

			/* Jedna iteracija slanja poruka */
			for (i = 0; i <= brojPaketa; i++)
			{
				/* Popunajvanje HEADER dela poruke */
				header->type = DATA;					/* Flag da se salju podaci */
				header->id = ++s->idPoslednjePoslato;	/* Dodeljuje se ID poruci */

				/* Racuna velicinu poruke */
				header->size = (brojPaketa != i && brojPaketa != 0) ? velicinaPoruke : (s->cwnd % velicinaPoruke);
				
				/* Pokusava da procita velicinu poruke sa buffera */
				trenutnoProcitano = rRead(s->sendBuffer, data, header->size);

				/* Ako na bufferu ima manje nego sto je trazeno, smanjuje se velicina poruke i broj poruka u iteraciji */
				if (trenutnoProcitano > s->sendBuffer->taken - procitano)
				{
					header->size = s->sendBuffer->taken - procitano;
					brojPaketa = i;
					s->brojPoslednjePoslatih = i + 1;
				}
				
				/* Povecava se ukupno procitano */
				procitano += trenutnoProcitano;

				/* Slanje poruke */
				iResult = sendto(s->socket, tempBuffer, sizeof(rMessageHeader) + header->size, 0, (LPSOCKADDR)(s->adresa), sizeof(sockaddr_in));
				
				printf("DBG: %d %x", header->id, *data);

				if (iResult == SOCKET_ERROR)
				{
					printf("\n SEND failed with error: %d\n", WSAGetLastError());
					continue;
				}

				ReleaseSemaphore(s->lock, 1, NULL);
				//printf("\n SEND: ID:[%d] B:[%d] ", header->id, header->size);	/* info */
			}
		}
		else
		{
			ReleaseSemaphore(s->lock, 1, NULL);
			Sleep(10);
			WaitForSingleObject(s->lock, INFINITE);
		}			
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

	WaitForSingleObject(s->lock, INFINITE);
	while (s->activeThreads)
	{
		if (s->adresa->sin_port == ntohs(0))
		{
			continue;
			ReleaseSemaphore(s->lock, 1, NULL);
		}

		ReleaseSemaphore(s->lock, 1, NULL);
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
				WaitForSingleObject(s->lock, INFINITE);
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
				ReleaseSemaphore(s->lock, 1, NULL);
				continue;
			}
		}

		WaitForSingleObject(s->lock, INFINITE);
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
			printf("DBG: %d %x", header->id, *data);

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

		ReleaseSemaphore(s->lock, 1, NULL);
		WaitForSingleObject(s->lock, INFINITE);
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

int Algoritam(rSocket* s)
{
	// Ako je primljeno manje nego poslato
	if (s->recv < s->cwnd)
	{
		// Ako je u slowstart modu
		if (s->slowstart)
		{
			// postavlja se ssthresh i cwnd = ssthresh
			s->ssthresh = s->cwnd / 2;
			s->cwnd = s->ssthresh;
			s->slowstart = false;
		}
		// Ako je u Tahoe modu
		else
		{
			// TREBA GA JOS MALO SMANJITI
			if (s->cwnd <= s->ssthresh + s->ssthresh / 10)
				s->ssthresh /= 2;
			// swnd se vraca na ssthresh
			s->cwnd = s->ssthresh;
		}
	}
	// Ako je primljeno isto kao poslato
	else
	{
		// Slowstart -> cwnd * 2
		if (s->slowstart)
			s->cwnd *= 2;
		// Tahoe -> cwnd + 1
		else
			s->cwnd += 1;
	}

	return 0;
}