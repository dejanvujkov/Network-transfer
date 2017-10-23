#include <WinSock2.h>

int Send(char* data, int len, SOCKET socket, LPSOCKADDR serveraddress, int addrLen);
int Recv(char* buffer, int len, SOCKET socket, LPSOCKADDR clientaddress, int addrLen);

typedef struct SendHelper {
	int slider;

	char* data;
	int datalen;

	SOCKET* clientsocket;
	LPSOCKADDR address;
	int addresslength;

	int cwnd;
	int ssthresh;
};