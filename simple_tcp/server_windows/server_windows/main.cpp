#pragma comment (lib, "ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>

#include <io.h>
#include <mutex>
#include <thread>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

SOCKET sockfd;

int const MAX_SECONDS_READ_TIMEOUT = 5;

int const MAX_CLIENT_COUNT = 10;
int totalActiveClientsCount = 0;
int isFinishing = FALSE;

std::mutex mutexTotalAcitveClientsCount;

void closeSocket(SOCKET socket) {
	printf("Closing socket = %d \r\n", socket);
	shutdown(socket, SD_BOTH);
	closesocket(socket);
}

void cleanUpClientConnectionThread(SOCKET newsockfd) {
	closeSocket(newsockfd);

	mutexTotalAcitveClientsCount.lock();
	printf("Handler starts deactivating for newsockfd = %d\r\n", newsockfd);
	totalActiveClientsCount--;
	printf("Total active clients = %d\r\n", totalActiveClientsCount);
	mutexTotalAcitveClientsCount.unlock();
}

DWORD WINAPI handleClientConnection(CONST LPVOID arg) {
	SOCKET newsockfd = *(SOCKET*)arg;

	struct timeval timeout;
	timeout.tv_sec = MAX_SECONDS_READ_TIMEOUT;
	timeout.tv_usec = 0;
	// On timeout read returns -1
	setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	char buffer[256];
	int totalBytesCount;
	int additionalBytesCount;
	int messageBytesLength;

	messageBytesLength = 0;

	while (!isFinishing) {
		// Start reading new message
		bzero(buffer, sizeof(buffer));

		totalBytesCount = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);

		if (totalBytesCount == -1) {
			continue;
		}
		else if (totalBytesCount <= 0) {
			cleanUpClientConnectionThread(newsockfd);
			return WSAGetLastError();
		}

		messageBytesLength = buffer[0];

		printf("Initially read [%ld] bytes of [%ld] messageBytesLength\r\n", totalBytesCount, messageBytesLength);

		while (messageBytesLength > totalBytesCount) {
			// Continue reading message to assemble full size message
			printf("Reading additional bytes from = %d\r\n", newsockfd);
			if (isFinishing) {
				cleanUpClientConnectionThread(newsockfd);
				return WSAGetLastError();
			}

			additionalBytesCount = recv(newsockfd, buffer + totalBytesCount, sizeof(buffer) - 1 - totalBytesCount, 0);

			if (additionalBytesCount == -1) {
				continue;
			}
			else if (additionalBytesCount <= 0) {
				cleanUpClientConnectionThread(newsockfd);
				return WSAGetLastError();
			}

			totalBytesCount += additionalBytesCount;

			printf("Read [%ld] bytes of [%ld] messageBytesLength\r\n", totalBytesCount, messageBytesLength);
		}

		printf("Read from = %d:\r\n%s", newsockfd, buffer + 1);

		/* Write a response to the client */
		bzero(buffer, sizeof(buffer));
		sprintf_s(buffer + 1, sizeof(buffer) - 1, "I got your message");
		buffer[0] = 18;
		totalBytesCount = send(newsockfd, buffer, buffer[0] + 1, 0);

		if (totalBytesCount < 0) {
			printf("ERROR ON SENDING BACK, DISCONNECTING\r\n");
			cleanUpClientConnectionThread(newsockfd);
			return WSAGetLastError();
		}
	}

	// Server is finishing -> cleaning up
	cleanUpClientConnectionThread(newsockfd);

	return 0;
}

DWORD WINAPI acceptNewClients(CONST LPVOID lpParam) {
	printf("Ready for accepting incomming connections, sockfd = %d\r\n", sockfd);

	SOCKET newsockfd;
	unsigned int clilen;
	struct sockaddr_in cli_addr;

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	while (!isFinishing) {
		if (totalActiveClientsCount == MAX_CLIENT_COUNT) {
			// Limit for maximum clients count, the printf below can produce infinite flood
			// printf("Connection limit reached, new user will hanging on connect\r\n");
			continue;
		}

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (int*)&clilen);
		if (newsockfd < 0) {
			printf("ERROR ON ACCEPT, SKIPPING\r\n");
			continue;
		}

		if (CreateThread(NULL, 0, handleClientConnection, &newsockfd, 0, NULL) == NULL) {
			printf("Error on creating client thread\r\n");
		}
		else {
			mutexTotalAcitveClientsCount.lock();
			totalActiveClientsCount++;
			printf("Creating handler for newsockfd = %d\r\n", newsockfd);
			printf("Total active clients = %d\r\n", totalActiveClientsCount);
			mutexTotalAcitveClientsCount.unlock();
		}
	}

	return 0;
}

void mainThreadCleanUp() {
	isFinishing = TRUE;

	printf("Started cleaning up \r\n");

	while (totalActiveClientsCount != 0) {
		// Wait...
		printf("There are some clients on the end, count = %d\r\n", totalActiveClientsCount);
		Sleep(1000);
	}

	printf("Deactivating server socket \r\n");
	closeSocket(sockfd);

	printf("Program finished\r\n");
	
	WSACleanup();

	exit(0);
}

int main() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		exit(1);
	}

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		WSACleanup();
		exit(1);
	}

	struct sockaddr_in serv_addr;
	uint16_t portno;

	/* Initialize socket structure */
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = 5001;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		closeSocket(sockfd);
		WSACleanup();
		exit(1);
	}

	// Creating second thread for handling incoming connections
	if (CreateThread(NULL, 0, &acceptNewClients, NULL, 0, NULL) == NULL) {
		printf("Error on creating worker thread\r\n");
	}

	// Reading server-side commands from terminal
	char command;

	while (TRUE) {
		command = getchar();
		if (command == 'q') {
			break;
		}
		else {
			// Do some stuff
		}
	}

	// Perfect moment for cleaning up
	mainThreadCleanUp();

	return 0;
}