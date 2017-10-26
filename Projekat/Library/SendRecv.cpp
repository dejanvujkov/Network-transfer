

#include "header.h"

int Send(rSocket sock, char* data, int len)
{

	/** INICIJALIZACIJA **/

	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));

	int sockAddrLen = sizeof(struct sockaddr);
	
	int iResult = 0;

	Inicijalizuj(h, &sock, data, len);

	
	/** CONNECT **/
	rMessageHeader header;
	header.id = REQUEST;
	header.size = len;

	iResult = sendto(*(h->socket),
		(char*)&header,
		sizeof(rMessageHeader),
		0,
		(LPSOCKADDR)(h->adresa),
		sockAddrLen);

	if (iResult == SOCKET_ERROR) {
		printf("Sendto failed with error: %d\n", WSAGetLastError());
		closesocket(*(h->socket));
		WSACleanup();
		return 1;
	}

	printf("Poslat zahtev za komunikaciju\n");


	//RecvFrom Server - ocekuje se Accepted
	iResult = recvfrom(*(h->socket),
		(char*)&header,
		sizeof(rMessageHeader),
		0,
		(LPSOCKADDR)(h->adresa),
		&sockAddrLen);

	if (iResult == SOCKET_ERROR)
	{
		printf("recvfrom failed with error: %d\n", WSAGetLastError());
		return 1;
	}

	if (header.id == ACCEPTED) {

		printf("Connected to server");
		h->state = CONNECTED;
	}
	else if (header.id == REJECTED)
	{
		printf("Server rejected connection");
		h->state = DISCONNECTED;
	}
	/** CONNECT **/


	//thread1 petlja koja uzima iz data i stavlja u buffer
	HANDLE thread1 = CreateThread(NULL, 0, &FromDataToBuffer, h, 0, NULL);
	//HANDLE thread3 = CreateThread(NULL, 0, &EMPTYBUFFER, h, 0, NULL);

	//thread2 petlja koja uzima iz buffer i salje preko mreze
	HANDLE thread2 = CreateThread(NULL, 0, &SendDataFromBuffer, h, 0, NULL);

	getchar();


	//t2

	// while end

	rFreeBuffer(&(h->buffer));
	free(h);

	return 0;
}




