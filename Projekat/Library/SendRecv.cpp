#include "header.h"

int Send(rSocket sock, char* data, int len)
{
	/** INICIJALIZACIJA **/

	rHelper* h;
	h = (rHelper*)malloc(sizeof(rHelper));
	
	int iResult = 0;

	Inicijalizuj(h, &sock, data, len);


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

int Send2(SOCKET socket, char * buffer, int size, int flag, sockaddr * adresa, int tolen)
{
	rHelper h;

	Initialize(&h, &socket, buffer, size, adresa, tolen);

	//thread1 petlja koja uzima iz data i stavlja u buffer
	HANDLE thread1 = CreateThread(NULL, 0, &FromDataToBuffer, &h, 0, NULL);

	//thread2 petlja koja uzima iz buffer i salje preko mreze
	HANDLE thread2 = CreateThread(NULL, 0, &SendDataFromBuffer, &h, 0, NULL);

	getchar();

	rFreeBuffer(&(h.buffer));

	return 0;
}
