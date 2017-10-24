#pragma once

typedef struct rHelper {
	int slider;
	int cwnd;
	int ssthresh;
	int recv;
	bool slowstart;
	rConnectionState state;
	Kruzni_Buffer buffer;
	HANDLE lock;
} rHelper;

typedef struct rSocket {
	char* addr;
	int port;
} rSocket;
