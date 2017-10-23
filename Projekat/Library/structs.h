#pragma once

typedef struct rHelper {
	int slider;
	int cwnd;
	int ssthresh;
	int recv;
	bool slowstart;
	rConnectionState state;
} rHelper;

typedef struct rSocket {
	char* addr;
	int port;
} rSocket;
