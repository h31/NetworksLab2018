#pragma comment (lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <cstdint>

#include <WS2tcpip.h>

//#define IMITATE_PARTLY_SENDING

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

SOCKET sockfd;

void closeApp() {
	printf("Closing socket\r\n");
	closesocket(sockfd);
	exit(0);
}

BOOL WINAPI ctrlCHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		closeApp();
	}

	return TRUE;
}

int main(int argc, char *argv[]) {
	SetConsoleCtrlHandler(ctrlCHandler, TRUE);

	WSADATA wsaData;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	if (argc < 3) {
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve the server address and port
	iResult = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Attempt to connect to an address until one succeeds
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		sockfd = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (sockfd == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Connect to server.
		iResult = connect(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(sockfd);
			sockfd = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (sockfd == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	char buffer[256];
	int n;
	int textInputSize = 0;

	while (1) {
		printf("Please enter the message: ");
		bzero(buffer, sizeof(buffer));
		fgets(buffer + 1, sizeof(buffer) - 2, stdin);

		if (strstr(buffer + 1, "\\q") != NULL) {
			closeApp();
		}

		textInputSize = strlen(buffer + 1);
		printf("Message size = %d\r\n", textInputSize);
		buffer[0] = textInputSize;

		/* Send message to the server */
#ifdef IMITATE_PARTLY_SENDING
		int firstPartSize = 4;
		n = write(sockfd, buffer, firstPartSize);
		sleep(2);
		n = send(sockfd, buffer + firstPartSize, textInputSize + 2 - firstPartSize, 0);
#else
		n = send(sockfd, buffer, textInputSize + 2, 0);
#endif

		if (n < 0) {
			perror("ERROR writing to socket");
			closeApp();
		}

		/* Now read server response */
		bzero(buffer, sizeof(buffer));
		n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

		if (n <= 0) {
			perror("ERROR reading from socket");
			closeApp();
		}

		int responseLength = buffer[0];

		while (responseLength > n) {
			int addedBytesCount = recv(sockfd, buffer + n, sizeof(buffer) - 1 - n, 0);

			if (addedBytesCount <= 0) {
				perror("ERROR reading from socket");
				closeApp();
			}

			n += addedBytesCount;
		}

		printf("%s\n", buffer + 1);
	}
}