#include "Inicijalizacija.h"

int main(int argc, char* argv[])
{
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

	// Main server loop
	do 
	{
		// Buffer za ukupnu poruku koja se prima

		// Adresa klijenta koji se povezuje
		sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof(sockaddr_in));
		
		clientInfo = (rClientMessage*)malloc(sizeof(clientInfo));

		//iResult = listen(serverSocket, SOMAXCONN);
		/*if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return 1;
		}*/

		// ACCEPT **
		iResult = recvfrom(serverSocket,
			(char*)&connectionBuffer,
			ACCESS_BUFFER_SIZE,
			0,
			(LPSOCKADDR)&clientAddress,
			&sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}

		char ipAddress[IP_ADDRESS_LEN];
		strcpy_s(ipAddress, sizeof(ipAddress), inet_ntoa(clientAddress.sin_addr));
		int clientPort = ntohs((u_short)clientAddress.sin_port);

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

			iResult = sendto(serverSocket, (char*)&connectionBuffer, sizeof(rMessageHeader), 0, (LPSOCKADDR)&clientAddress, sockAddrLen);

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
		HANDLE thread = CreateThread(NULL, 0, &RecieveMessage, /*PARAMETAR STRUCT*/ clientInfo, 0, NULL);


	} while (1);

	Close(iResult, serverSocket);

	return 0;
}

int Close(int iResult, SOCKET serverSocket) {

	SOCKET clientSocket;

	iResult = closesocket(serverSocket);
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
}

DWORD WINAPI RecieveMessage(LPVOID param)
{
	rClientMessage* clientInfo = (rClientMessage*)param;

	int iResult;

	SOCKET clientSocket; // INIT

	rMessageHeader* header = (rMessageHeader*)clientInfo->buffer;
	char* message = clientInfo->buffer + sizeof(rMessageHeader);
	// NEBITNO int messageSize = header->size;

	// Buffer za svaku poruku
	char* accessBuffer;
	accessBuffer = (char*)malloc(ACCESS_BUFFER_SIZE);
	// NEBITNO memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);
	
	// Prima svaki paket
	while (clientInfo->messageSize - clientInfo->slider != 0)
	{
		/*iResult = recvfrom(serverSocket, accessBuffer, ACCESS_BUFFER_SIZE, 0, (LPSOCKADDR)&clientAddress, &sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}

		printf("\n[%d] Recieved %d ", header->id, header->size);
		memcpy(message, messageBuffer, header->size);
		slider += header->size;

		printf("%d do sada", slider);

		header->state = RECIEVED;

		iResult = sendto(serverSocket,
			accessBuffer,
			sizeof(rMessageHeader),
			0,
			(LPSOCKADDR)&clientAddress,
			sockAddrLen);

		if (iResult == SOCKET_ERROR)
		{
			printf("recvfrom failed with error: %d\n", WSAGetLastError());
			continue;
		}*/
	}

	free(clientInfo);
}