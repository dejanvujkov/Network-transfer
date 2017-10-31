#include "Inicijalizacija.h"

DWORD WINAPI RecieveMessage(LPVOID param);

int main(int argc, char* argv[])
{
	//HANDLE lock = CreateSemaphore(0, 1, 1, NULL);

	bool lock = true;
	DWORD timeout = 2 * 1000;
	SOCKET serverSocket;

	sockaddr_in serverAddress;
	int sockAddrLen = sizeof(struct sockaddr);

	int iResult;
	rMessageHeader connectionBuffer;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Initialize serverAddress structure used by bind
	memset((char*)&serverAddress, 0, sizeof(serverAddress));

	InitializeSocket(&serverSocket, &serverAddress);

	// Bind port number and local address to socket
	iResult = bind(serverSocket, (LPSOCKADDR)&serverAddress, sizeof(serverAddress));

	if (iResult == SOCKET_ERROR)
	{
		printf("Socket bind failed with error: %d\n", WSAGetLastError());
		closesocket(serverSocket);
		WSACleanup();
		return 1;
	}

	setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	printf("Simple UDP server started and waiting client messages.\n");

	rClientMessage* clientInfo;
	sockaddr_in* clientAddress;

	// Main server loop
	while(1)
	{
		// Buffer za ukupnu poruku koja se prima
		
		// Adresa klijenta koji se povezuje
		clientAddress = (sockaddr_in*)malloc(sizeof(sockaddr_in));
		if (clientAddress == NULL)
			continue;
		memset(clientAddress, 0, sizeof(sockaddr_in));
		
		clientInfo = (rClientMessage*)malloc(sizeof(rClientMessage));
		if (clientInfo == NULL)
			continue;

		do
		{
			if (lock)
				break;
			Sleep(1000);
		} while (1);

		//ReleaseSemaphore(lock, 1, 0);
		// ACCEPT **
		iResult = recvfrom(serverSocket,
			(char*)&connectionBuffer,
			sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)clientAddress,
			&sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			if (WSAGetLastError() == 10060) {
				printf("Timed out\n");
				continue;
			}
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}

		char ipAddress[IP_ADDRESS_LEN];
		strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa((*clientAddress).sin_addr));
		int clientPort = ntohs((u_short)(*clientAddress).sin_port);

		printf("Client connected from ip: %s, port: %d\n", ipAddress, clientPort);
		
		
		if (connectionBuffer.state == REQUEST)
		{
			char* messageBuffer = (char*)malloc(connectionBuffer.size);

			if (messageBuffer != NULL)
				connectionBuffer.state = ACCEPTED;
			else
				connectionBuffer.state = REJECTED;

			clientInfo->buffer = messageBuffer;
			clientInfo->clientAddress = clientAddress;
			clientInfo->messageSize = connectionBuffer.size;
			clientInfo->slider = 0;
			clientInfo->socket = serverSocket;
			clientInfo->lock = &lock;

			iResult = sendto(serverSocket, (char*)&connectionBuffer, sizeof(rMessageHeader), 0, (LPSOCKADDR)clientAddress, sockAddrLen);

			if (iResult == SOCKET_ERROR) {
				printf("Sendto failed with error: %d\n", WSAGetLastError());
				closesocket(serverSocket);
				free(messageBuffer);
				WSACleanup();
				return 1;
			}

			if (connectionBuffer.state == ACCEPTED)
			{
				printf("Server poslao Accepted\n");
			}
			else
			{
				printf("Server poslao Rejected\n");
				continue;
			}
		}
		else
		{
			// Ako nije REQUEST server ignorise poruku
			continue;
		}
		// ACCEPT END **


		// Primi celu poruku

		lock = false;
		HANDLE thread = CreateThread(NULL, 0, RecieveMessage, clientInfo, 0, NULL);
		//WaitForSingleObject(lock, INFINITE);
		//Sleep(100);
		
	}// while (1);

	Close(serverSocket);

	free(clientAddress);
	free(clientInfo);

	return 0;
}

int Close(SOCKET serverSocket) {

	int iResult = closesocket(serverSocket);
	if (iResult == SOCKET_ERROR)
	{
		printf("closesocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	iResult = WSACleanup();
	if (iResult == SOCKET_ERROR)
	{
		printf("WSACleanup failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	printf("Server successfully shut down.\n");

	return 0;
}

DWORD WINAPI RecieveMessage(LPVOID param)
{
	rClientMessage* clientInfo = (rClientMessage*)param;
	//WaitForSingleObject(clientInfo->lock, INFINITE);

	int id = 1;

	int iResult;

	int sockAddrLen = sizeof(struct sockaddr);

	// Buffer za svaku poruku
	char* accessBuffer;
	accessBuffer = (char*)malloc(ACCESS_BUFFER_SIZE);
	if (accessBuffer == NULL)
		return 1;

	rMessageHeader* header = (rMessageHeader*)accessBuffer;
	char* message = accessBuffer + sizeof(rMessageHeader);
	// NEBITNO memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
	
	// Prima svaki paket
	while (clientInfo->messageSize - clientInfo->slider != 0)
	{
		iResult = recvfrom(clientInfo->socket, accessBuffer, ACCESS_BUFFER_SIZE, 0, (LPSOCKADDR)clientInfo->clientAddress, &sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("\nrecvfrom failed with error: %d", WSAGetLastError());
			continue;
		}

		if (header->state == REQUEST)
		{
			// Ako stigne request od drugog klijenta, salje se reject
			header->state = REJECTED;

			iResult = sendto(clientInfo->socket,
				accessBuffer,
				sizeof(rMessageHeader),
				0,
				(LPSOCKADDR)clientInfo->clientAddress,
				sockAddrLen);

			continue;
		}

		if (header->id != id)
			header->state = DROPPED;
		else
			header->state = RECIEVED;

		iResult = sendto(clientInfo->socket,
			accessBuffer,
			sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)clientInfo->clientAddress,
			sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("\nrecvfrom failed with error: %d", WSAGetLastError());
			continue;
		}

		if (header->id != id) 
		{
			printf("\n[%d] dropped, expected [%d]", header->id, id);
			continue;
		}

		id++;
		printf("\n[%d] Recieved: %d ", header->id, header->size);
		memcpy(message, clientInfo->buffer, header->size);
		clientInfo->slider += header->size;

		printf("\t\tTotal: %d", clientInfo->slider);
	}

	*(clientInfo->lock) = true;
	//ReleaseSemaphore(*(clientInfo->lock), 1, NULL);
	free(clientInfo->clientAddress);
	//free(clientInfo);
	free(accessBuffer);

	free(clientInfo->buffer);

	return 0;
}