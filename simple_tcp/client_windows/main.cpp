#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#pragma warning(disable:4996)

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORTION 9
#define DEFAULT_PORT "27015"

int readn(SOCKET sock, char* buf, int buflen, int _n)
{
	ZeroMemory(buf, buflen);
	int iResult;
	int n = 0;
	while (n < _n)
	{
		iResult = recv(sock, buf + n, _n - n, 0);
		if (iResult > 0) {
			n += iResult;
		}
		else if (iResult == 0)
		{
			printf("connection terminated by remote user...\n");
			return SOCKET_ERROR;
		}
		else {
			printf("recv failed with error: %d\nEXITING\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
	}
	return n;
}

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char sendbuf[DEFAULT_BUFLEN] = { 0 };
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;

	const char* server_name = "localhost";

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		getchar();
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(server_name, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		getchar();
		return 1;
	}

	printf("connecting to %s:%s...\n", server_name, DEFAULT_PORT);
	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			getchar();
			return 1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		getchar();
		return 1;
	}

	int n_total = 0;
	int n_expected = 0;

	printf("connected to %s:%s...\n", server_name, DEFAULT_PORT);
	while (true)
	{
		gets_s(sendbuf, DEFAULT_BUFLEN);
		if (strlen(sendbuf) != 0)
		{
			iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ConnectSocket);
				WSACleanup();
				getchar();
				return 1;
			}

			printf("Bytes Sent: %ld\n", iResult);

			ZeroMemory(recvbuf, sizeof(recvbuf));
			
			n_total += iResult;
			while (n_total >= DEFAULT_PORTION)
			{
				n_total -= DEFAULT_PORTION;
				n_expected += DEFAULT_PORTION;
			}

			while (n_expected > 0)
			{
				iResult = readn(ConnectSocket, recvbuf, DEFAULT_BUFLEN, DEFAULT_PORTION);
				if (iResult == SOCKET_ERROR) {
					printf("readn failed with error: %d\nEXITING\n", WSAGetLastError());
					closesocket(ConnectSocket);
					WSACleanup();
					getchar();
					return 1;
				}
				printf("Received:'%s' (%d bytes)\n", recvbuf, iResult);
				n_expected -= 9;
			}
			
		}
		else break;
	}
	

	// shutdown the connection since no more data will be sent
	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ConnectSocket);
		WSACleanup();
		getchar();
		return 1;
	}

	// Receive until the peer closes the connection
	do {
		ZeroMemory(recvbuf, sizeof(recvbuf));
		iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
			printf("Bytes received: %d\n", iResult);
		else if (iResult == 0)
			printf("connection closed\n");
		else
			printf("recv failed with error: %d\n", WSAGetLastError());

	} while (iResult > 0);

	// cleanup
	closesocket(ConnectSocket);
	WSACleanup();

	getchar();
	return 0;
}
