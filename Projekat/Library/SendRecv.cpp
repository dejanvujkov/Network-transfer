#include "header.h"

int Send(rSocket sock, char* data, int len)
{
	/** INICIJALIZACIJA **/

	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));

	int sockAddrLen = sizeof(struct sockaddr);
	
	int iResult = 0;

	Inicijalizuj(h, &sock, data, len);

	if ((iResult = KonektujSe(h, len)) == -1) {
		
		printf("Doslo je do greske prilikom konektovanja na server\n");
		return -1;
	}

	//thread1 petlja koja uzima iz data i stavlja u buffer
	HANDLE thread1 = CreateThread(NULL, 0, &FromDataToBuffer, h, 0, NULL);
	//HANDLE thread3 = CreateThread(NULL, 0, &EMPTYBUFFER, h, 0, NULL);

	//thread2 petlja koja uzima iz buffer i salje preko mreze
	HANDLE thread2 = CreateThread(NULL, 0, &SendDataFromBuffer, h, 0, NULL);

	getchar();

	rFreeBuffer(&(h->buffer));
	free(h);

	return 0;
}