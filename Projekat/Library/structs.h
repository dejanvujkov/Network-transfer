#pragma once
/**
 * Strukture koje se koriste u projektu
 */

/* Struktura za upravljanje kruznim bufferom */
typedef struct Kruzni_Buffer
{
	char* buffer_start;		/* Pokazivac na pocetak buffera */
	char* buffer_end;		/* Pokazivac na kraj buffera */
	char* head;				/* Pokazivac na lokaciju za pisanje */
	char* tail;				/* Pokazivac na lokaciju za citanje */
	int taken;				/* Velicina zauzete memorije */
	int free;				/* Velicina slobodne memorije */
} Kruzni_Buffer;

/* Socket struktura za upravljanje konekcijom */
typedef struct rSocket
{
	HANDLE* sendThread;		/* Thread za slanje podataka */
	HANDLE* recvThread;		/* Thread za primanje podataka */
	bool activeThreads;		/* Flag da li su threadovi aktivni */

	HANDLE lock;			/* Kljuc za socket strukturu */

	rConnectionState state; /* Stanje konekcije sa klijentom/serverom */
	int sockAddrLen;		/* Duzina adrese */
	SOCKET socket;			/* UDP Socket */
	sockaddr_in* adresa;	/* Adresa klijenta/servera */

	int cwnd;				/* Broj bajtova koji se salju u svakoj iteraciji */
	int ssthresh;			/* Minimalni bezbedni broj bajtova */
	int recv;				/* Ukupno dostavljeno bajtova */
	bool slowstart;			/* True - SlowStart mod | False - Tahoe mod */

	Kruzni_Buffer* sendBuffer;	/* Kruzni_Buffer za slanje podataka */
	Kruzni_Buffer* recvBuffer;  /* Kruzni_Buffer za pirmanje podataka */

	int brojPoslednjePoslatih;	/* Broj poslatih paketa u jednoj iteraciji */
	int idPoslednjePoslato;		/* ID poslednje poslatog paketa */
	int idOcekivanog;			/* ID paketa koji se ocekuje */
	bool timedOut;				/* Flag da li je doslo do TIMEOUT-a */
	bool canSend;				/* True - Stigli ACK | False - Ocekuju se ACK */
} rSocket;

/* Header svakog UPD paketa, sluzi i za slanje Tip poruka */
typedef struct rMessageHeader {
	rMessageType type;		/* Tip poruke */
	int id;					/* ID poruke */
	int size;				/* Velicina podataka iza headera */
} rMessageHeader;
