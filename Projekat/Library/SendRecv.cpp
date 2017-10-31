#include "header.h"

int Send(SOCKET socket, char * buffer, int size, int flag, sockaddr * adresa, int tolen)
{
	rHelper h;

	Initialize(&h, &socket, buffer, size, adresa, tolen);

	//thread1 petlja koja uzima iz data i stavlja u buffer
	HANDLE thread1 = CreateThread(NULL, 0, &FromDataToBuffer, &h, 0, NULL);

	//thread2 petlja koja uzima iz buffer i salje preko mreze
	HANDLE thread2 = CreateThread(NULL, 0, &SendDataFromBuffer, &h, 0, NULL);

	WaitForSingleObject(thread1, INFINITE);
	WaitForSingleObject(thread2, INFINITE);
	
	printf("\nPoruka poslata");
	
	return 0;
}


int Receive(SOCKET socket, int messageLength, bool* lock, sockaddr_in* clientAddress, rClientMessage* info) {

	char* messageBuffer = (char*)malloc(messageLength);

	//Popunjavanje odgovarajucih polja
	info->buffer = messageBuffer;
	info->messageSize = messageLength;
	info->slider = 0;
	info->socket = socket;
	info->clientAddress = clientAddress;
	info->lock = lock;

	// Primi celu poruku
	*lock = false;
	HANDLE thread = CreateThread(NULL, 0, RecieveMessage, info, 0, NULL);

	WaitForSingleObject(thread, INFINITE);

	return 0;
}