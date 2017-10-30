#include "Inicijalizacija.h"

int main(int argc, char* argv[])
{
	sockaddr_in serverAddress;
	int sockAddrLen = sizeof(struct sockaddr);
	int iResult;
	SOCKET serverSocket;

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

	// Main server loop
	while (1)
	{
		char* messageBuffer;
		int slider = 0;
		int messageSize = 0;
		char* accessBuffer;
		accessBuffer = (char*)malloc(ACCESS_BUFFER_SIZE);
		memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);

		rMessageHeader* header;
		char* message;

		sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof(sockaddr_in));
				

		// receive client message
		iResult = recvfrom(serverSocket,
			accessBuffer,
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
		
		header = (rMessageHeader*)accessBuffer;
		message = accessBuffer + sizeof(rMessageHeader);
		messageSize = header->size;
		
		if (header->id == REQUEST) {
						
			messageBuffer = (char*)malloc(messageSize);

			if (messageBuffer != NULL)
			{
				//sendto "Accepted" Clinet
				header->id = ACCEPTED;

				iResult = sendto(serverSocket, accessBuffer, sizeof(rMessageHeader), 0, (LPSOCKADDR)&clientAddress, sockAddrLen);

				if (iResult == SOCKET_ERROR) {
					printf("Sendto failed with error: %d\n", WSAGetLastError());
					closesocket(serverSocket);
					WSACleanup();
					return 1;
				}
				
				printf("Server poslao Accepted\n");
			}
			else
			{
				//sendto "Rejected" Clinet
				header->id = REJECTED;

				iResult = sendto(serverSocket, accessBuffer, sizeof(rMessageHeader), 0, (LPSOCKADDR)&clientAddress, sockAddrLen);

				if (iResult == SOCKET_ERROR) {
					printf("Sendto failed with error: %d\n", WSAGetLastError());
					closesocket(serverSocket);
					WSACleanup();
					return 1;
				}

				printf("Server poslao Rejected\n");
			}
		}
		else
		{
			//sendto "Rejected" Clinet
			header->id = REJECTED;
			iResult = sendto(serverSocket, accessBuffer, sizeof(rMessageHeader), 0, (LPSOCKADDR)&clientAddress, sockAddrLen);

			if (iResult == SOCKET_ERROR) {
				printf("Sendto failed with error: %d\n", WSAGetLastError());
				closesocket(serverSocket);
				WSACleanup();
				return 1;
			}

			printf("Server poslao Rejected\n");
		}

		while (messageSize - slider != 0)
		{
			iResult = recvfrom(serverSocket, accessBuffer, ACCESS_BUFFER_SIZE, 0, (LPSOCKADDR)&clientAddress, &sockAddrLen);

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
			}
		}
	}

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
}
