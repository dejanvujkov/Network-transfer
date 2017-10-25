#pragma once

typedef struct rHelper {
	int length;
	char* data;
	int slider;
	int cwnd;
	int ssthresh;
	int recv;
	bool slowstart;
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
