#pragma once

typedef struct Kruzni_Buffer
{
	char* buffer_start;
	char* buffer_end;
	char* head;
	char* tail;
	int taken;
	int free;
}Kruzni_Buffer;

typedef struct rHelper {
	int length;
	char* data;
	int slider;
	int cwnd;
	int ssthresh;
	int recv;
	bool slowstart;
	int sockAddrLen;
	rConnectionState state;
	Kruzni_Buffer buffer;
	HANDLE lock;

	SOCKET* socket;
	sockaddr_in* adresa;
} rHelper;

typedef struct rSocket {
	char* addr;
	int port;
} rSocket;

typedef struct rMessageHeader {
	rMessageState state;
	int id;
	int size;
};