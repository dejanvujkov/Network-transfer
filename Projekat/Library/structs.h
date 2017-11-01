#pragma once

typedef struct Kruzni_Buffer
{
	char* buffer_start;		// Pokazivac na pocetak buffera
	char* buffer_end;		// Pokazivac na kraj buffera
	char* head;				// Pokazivac za pisanje
	char* tail;				// Pokazivac za citanje
	int taken;				// Duzina zauzete memorije
	int free;				// Duzina slobodne memorije
}Kruzni_Buffer;


typedef struct rSocket
{
	// Threads
	HANDLE* sendThread;
	HANDLE* recvThread;
	bool activeThreads;

	// Connection
	rConnectionState state;
	int sockAddrLen;
	SOCKET socket;
	sockaddr_in* adresa;

	// Algoritam
	int cwnd;
	int ssthresh;
	int recv;
	bool slowstart;

	// Bufferi
	Kruzni_Buffer* sendBuffer;
	Kruzni_Buffer* recvBuffer;

	//
	int brojPoslednjePoslatih;
	int brojPoslednjePrimljenih;
	int idOcekivanog;
	bool timedOut;
	bool canSend;


} rSocket;

typedef struct rMessageHeader {
	rMessageType type;		// Stanje poruke
	int id;					// Oznaka poruke
	int size;				// Velicina poruke
} rMessageHeader;


////////////////
//
//
//typedef struct rHelper {
//	char* data;				// Pokazivac na podatke
//	int length;				// Duzina podataka za slanje
//	int slider;				// Duzina podataka prebacenih na buffer
//
//	int cwnd;				// Duzina trenutne poruke			// Algoritam
//	int ssthresh;			// Minimalna sigurna duzina poruke  // Algoritam
//	int recv;				// Duzina primljene poruke			// Algoritam
//	bool slowstart;			// Slowstart = true / Tahoe = false	// Algoritam
//
//	rConnectionState state;	// Stanje konekcije
//	Kruzni_Buffer buffer;	// Kruzni buffer za smestanje poruke
//	HANDLE lock;			// Lock objekat za zakljucavanje strukture
//	
//	int sockAddrLen;		// Duzina adrese
//	SOCKET* socket;			// SOCKET za slanje
//	sockaddr_in* adresa;	// Adresa servera
//} rHelper;
//
//typedef struct rClientMessage {
//	char* buffer;
//	int slider;
//	int messageSize;
//	sockaddr_in* clientAddress;
//	SOCKET socket;
//	bool* lock;
//} rClientMessage;
