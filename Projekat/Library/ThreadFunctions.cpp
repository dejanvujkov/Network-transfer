#include "header.h"

DWORD WINAPI SendThread(LPVOID param)
{
	/*  */
	rSocket* s = (rSocket*)param;

	int iResult;		/* Flag greske */	
	char* tempBuffer = (char*)malloc(MAX_UDP_SIZE);				/* Buffer za UDP poruku */
	
	rMessageHeader* header = (rMessageHeader*)tempBuffer;		/* Pokazivac na HEADER deo poruke */	
	char* data = tempBuffer + sizeof(rMessageHeader);			/* Pokazivac na DATA deo poruke */

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
			brojPaketa = (s->cwnd) / velicinaPoruke;				/* Racunanje broja poruka u iteraciji */
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

	/* Free buffer koriscen za primanje poruka */
	free(tempBuffer);

	return 0;
}

DWORD WINAPI RecvThread(LPVOID param)
{
	rSocket* s = (rSocket*)param;
	
	int iResult;		/* Flag greske */
	char* tempBuffer = (char*)malloc(MAX_UDP_SIZE);						/* Buffer za UDP poruku */
		
	rMessageHeader* header = (rMessageHeader*)tempBuffer;				/* Pokazivac na HEADER deo poruke */	
	char* data = tempBuffer + sizeof(rMessageHeader);					/* Pokazivac na DATA deo poruke */
	
	/* Pomocni podaci za upravljanje paketima */
	char* ackBuffer = (char*)malloc(2000 * sizeof(rMessageHeader));		/* Buffer za ACK poruke */	
	int ackCount = 0;													/* Brojac ACK poruka */

	WaitForSingleObject(s->lock, INFINITE);
	/* Thread ostaje aktivan dokle god je rSocket::activeThreads = true */
	while (s->activeThreads)
	{
		/* Ako rAccept nije zavrsio */
		if (s->adresa->sin_port == ntohs(0))
		{
			continue;
			ReleaseSemaphore(s->lock, 1, NULL);
		}

		ReleaseSemaphore(s->lock, 1, NULL);
		/* Oslobadja semafor dok ceka na poruku */
		iResult = recvfrom(s->socket, tempBuffer, MAX_UDP_SIZE, 0, (LPSOCKADDR)s->adresa, &(s->sockAddrLen));
		
		/* Recv ERROR */
		if (iResult == SOCKET_ERROR)
		{
			iResult = WSAGetLastError();
			/* Ako je bilo koja greska osim Timeout */
			if (iResult != 10060)
			{
				//printf("\nrecvfrom failed with error: %d\n", iResult);
				continue;
			}
			/* Ako je Timeout */
			else
			{
				printf("\n-- TIME OUT --");
				WaitForSingleObject(s->lock, INFINITE);

				/* Ako ima ACK-ova za obradjivanje */
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

		WaitForSingleObject(s->lock, INFINITE);

		/* Gleda tip primljene poruke */
		switch (header->type)
		{
		case REQUEST:
		{
			/* Ukoliko jos nije uspostavljena veza */
			if (s->state == DISCONNECTED)
			{
				header->type = ACCEPT;
				header->id = 0;
				header->size = 0;

				s->state = CONNECTED;
			}
			/* Ukoliko je veza uspostavljena sa nekim */
			else
			{
				header->type = REJECT;
				header->id = 0;
				header->size = 0;
			}

			/* Slanje odgovora klijentu */
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
		/* Ako su primljeni podaci */
		case DATA:
		{
			if (s->state == DISCONNECTED)
				continue;

			/* Povecava recv kruzni buffer dok ne bude dovoljno velik da primi poruku */
			while (s->recvBuffer->free < header->size)
			{
				iResult = rResize(s->recvBuffer, (s->recvBuffer->buffer_end - s->recvBuffer->buffer_start) * 2);
				if (iResult != 0)
				{
					printf("\nBUFFER NE MOZE DA SE UVECA");
					continue;
				}
			}

			/* Stavlja poruku na buffer */
			rPush(s->recvBuffer, data, header->size);

			printf("\n [%d] Recieved B: %d ", header->id, header->size);

			/* Menja tip header-a poruke */
			header->type = ACK;

			/* Salje ACK sa podacima primljene poruke */
			iResult = sendto(s->socket, (char*)header, sizeof(rMessageHeader), 0, (LPSOCKADDR)s->adresa, s->sockAddrLen);
			if (iResult == SOCKET_ERROR)
			{
				printf("DATA ACK-> recvfrom failed with error: %d\n", WSAGetLastError());
				continue;
			}
			break;
		}
		/* Ako je primljen ACK */
		case ACK:
		{
			if (s->state == DISCONNECTED)
				continue;
			
			if (s->timedOut)
				continue;

			/* Smesta ACK u buffer i povecava count za 1 */
			memcpy(ackBuffer + ackCount * sizeof(rMessageHeader), header, sizeof(rMessageHeader));
			ackCount++;

			printf("\nACK [%d]", header->id);

			/* Ako su primljeni svi ACK-ovi bice obradjeni bez cekanja Timeout-a */
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

	/* Oslobadjanje zauzetih buffera */
	free(tempBuffer);
	free(ackBuffer);

	return 0;
}

void CountACKs(rSocket* s, char* ackBuffer, int ackCount)
{
	rMessageHeader* header;

	/* Prolazi kroz sve ACK poruke */
	for (int i = 0; i < ackCount; i++)
	{
		for (int j = 0; j < ackCount + 1; j++)
		{
			header = (rMessageHeader*)(ackBuffer + j * sizeof(rMessageHeader));
			/* Ako nije pronasao ACK ocekivanog paketa odbacuje sve preostale */
			if (j == ackCount + 1)
				return;

			/* Ako je pronadjen ACk ocekivanog paketa, obradjuje se */
			if (header->id == s->idOcekivanog)
			{
				s->recv += header->size;
				rDelete(s->sendBuffer, header->size);
				s->idOcekivanog++;
				break;
			}
		}
	}

	/* Podesava ID poslednje poslatog da bude poslednji uspesno obradjen */
	s->idPoslednjePoslato = s->idOcekivanog - 1;
}

int Algoritam(rSocket* s)
{
	/* Ako je doslo do gubitka podataka */
	if (s->recv < s->cwnd)
	{
		/* Ako je u SlowStart modu smanjuje cwnd 2x i postavlja ssthresh*/
		if (s->slowstart)
		{
			// postavlja se ssthresh i cwnd = ssthresh
			s->ssthresh = s->cwnd / 2;
			s->cwnd = s->ssthresh;
			s->slowstart = false;
		}
		/* Ako je u Tahoe modu vraca cwnd na ssthresh*/
		else
		{
			/* *DODATAK* Ako nije mnogo odmakao od poslednjeg smanjenja, smanjuje ga opet  */
			if (s->cwnd <= s->ssthresh + s->ssthresh / 50)	// 100 - 102 % od ssthresh
				s->ssthresh /= 2;
			/* Vraca cwnd na ssthresh */
			s->cwnd = s->ssthresh;
		}
	}
	/* Ako je primljeno koliko je ocekivano */
	else
	{
		/* Ako je u SlowStart modu povecava cwnd 2x*/
		if (s->slowstart)
			s->cwnd *= 2;
		/* Ako je u Tahoe modu povecava cwnd za 1*/
		else
			s->cwnd += 1;
	}

	return 0;
}