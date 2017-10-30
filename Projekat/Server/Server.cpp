#include "Inicijalizacija.h"

DWORD WINAPI RecieveMessage(LPVOID param);

int main(int argc, char* argv[])
{
	//HANDLE lock = CreateSemaphore(0, 1, 1, NULL);

	bool lock = true;

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

	printf("Simple UDP server started and waiting client messages.\n");

	rClientMessage* clientInfo;
	sockaddr_in* clientAddress;

	// Main server loop
	while(1)
	{
		// Buffer za ukupnu poruku koja se prima

		// Adresa klijenta koji se povezuje
		clientAddress = (sockaddr_in*)malloc(sizeof(sockaddr_in));
		memset(clientAddress, 0, sizeof(sockaddr_in));
		
		clientInfo = (rClientMessage*)malloc(sizeof(rClientMessage));

		//iResult = listen(serverSocket, SOMAXCONN);
		/*if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}*/


		//WaitForSingleObject(lock, INFINITE);
		do
		{
			if (lock)
				break;
		} while (1);

		// ACCEPT **
		iResult = recvfrom(serverSocket,
			(char*)&connectionBuffer,
			sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)clientAddress,
			&sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}

		char ipAddress[IP_ADDRESS_LEN];
		strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa((*clientAddress).sin_addr));
		int clientPort = ntohs((u_short)(*clientAddress).sin_port);

		printf("Client connected from ip: %s, port: %d\n", ipAddress, clientPort);
		
		
		if (connectionBuffer.id == REQUEST)
		{
			char* messageBuffer = (char*)malloc(connectionBuffer.size);

			if (messageBuffer != NULL)
				connectionBuffer.id = ACCEPTED;
			else
				connectionBuffer.id = REJECTED;

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
				WSACleanup();
				return 1;
			}

			if (connectionBuffer.id == ACCEPTED)
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
		//Sleep(100);
		
	}// while (1);

	Close(serverSocket);

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

	int iResult;

	int sockAddrLen = sizeof(struct sockaddr);

	// Buffer za svaku poruku
	char* accessBuffer;
	accessBuffer = (char*)malloc(ACCESS_BUFFER_SIZE);

	rMessageHeader* header = (rMessageHeader*)accessBuffer;
	char* message = accessBuffer + sizeof(rMessageHeader);
	// NEBITNO memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
	
	// Prima svaki paket
	while (clientInfo->messageSize - clientInfo->slider != 0)
	{
		iResult = recvfrom(clientInfo->socket, accessBuffer, ACCESS_BUFFER_SIZE, 0, (LPSOCKADDR)clientInfo->clientAddress, &sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}

		printf("\n[%d] Recieved %d ", header->id, header->size);
		memcpy(message, clientInfo->buffer, header->size);
		clientInfo->slider += header->size;

		printf("%d do sada", clientInfo->slider);

		header->state = RECIEVED;

		iResult = sendto(clientInfo->socket,
			accessBuffer,
			sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)clientInfo->clientAddress,
			sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}
	}

	*(clientInfo->lock) = true;
	//ReleaseSemaphore(*(clientInfo->lock), 1, NULL);
	free(clientInfo->clientAddress);
	//free(clientInfo);
	return 0;
}