#include "header.h"

int Send(SOCKET socket, char * buffer, int size, int flag, sockaddr * adresa, int tolen)
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

int Recv(SOCKET socket, char * buffer, int size, LPSOCKADDR * clientaddress, int addrLen)
{
	return 0;
}
