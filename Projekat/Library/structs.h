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
