#include "Server.h"
#include "../Library/header.h"
int main(int argc, char* argv[])
{
	sockaddr_in serverAddress;
	int serverPort = SERVER_PORT;
	int sockAddrLen = sizeof(struct sockaddr);
	int iResult;

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// Initialize serverAddress structure used by bind
	memset((char*)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET; /*set server address protocol family*/
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(serverPort);

	// create a socket
	SOCKET serverSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_DGRAM,   // datagram socket
		IPPROTO_UDP); // UDP

					  // check if socket creation succeeded
	if (serverSocket == INVALID_SOCKET)
	{
		printf("Creating socket failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

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
		char* accessBuffer;
		char* messageBuffer;
		accessBuffer = (char*)malloc(ACCESS_BUFFER_SIZE);
		memset(accessBuffer, 0, ACCESS_BUFFER_SIZE);

		sockaddr_in clientAddress;
		memset(&clientAddress, 0, sizeof(sockaddr_in));
				

		// receive client message
		iResult = recvfrom(serverSocket,
			accessBuffer,
			2*sizeof(int),
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

		printf("Client connected from ip: %s, port: %d, sent: %s.\n", ipAddress, clientPort, accessBuffer);
		
		/*for (int i = 0; i < 4; i++)
			printf("  %x  ", accessBuffer[i]);*/
		
		if (*(int*)accessBuffer == REQUEST) {
						
			messageBuffer = (char*)malloc(*(int*)accessBuffer + 1);

			if (messageBuffer != NULL)
			{
				//sendto "Accepted" Clinet
				*(int*)accessBuffer = ACCEPTED;
				iResult = sendto(serverSocket,
					accessBuffer,
					sizeof(int),
					0,
					(LPSOCKADDR)&clientAddress,
					sockAddrLen);

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
				*(int*)accessBuffer = REJECTED;
				iResult = sendto(serverSocket,
					accessBuffer,
					sizeof(int),
					0,
					(LPSOCKADDR)&clientAddress,
					sockAddrLen);

				if (iResult == SOCKET_ERROR) {
					printf("Sendto failed with error: %d\n", WSAGetLastError());
					closesocket(serverSocket);
					WSACleanup();
					return 1;
				}

				printf("Server poslao Rejected\n");
			}
		}

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
		
		rMessageHeader* header;
		char* message;
		header = (rMessageHeader*)accessBuffer;
		message = accessBuffer + sizeof(rMessageHeader);

		for (int i = 0; i < header->size; i++)
		printf("  %x  ", message[i]);

		// possible message processing logic could be placed here
	}

	// if we are here, it means that server is shutting down
	// close socket and unintialize WinSock2 library
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
	return 0;
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}
