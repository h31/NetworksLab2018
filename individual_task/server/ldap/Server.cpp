#include "Server.h"
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)  
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

Server::Server(int port, int maxClientsCount, int readTimeoutInMilliseconds) 
	: port(port), maxClientsCount(maxClientsCount), readTimeoutInMilliseconds(readTimeoutInMilliseconds) { }

void Server::start() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		exit(1);
	}

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0) {
		WSACleanup();
		exit(1);
	}

	struct sockaddr_in serv_addr;
	uint16_t portno;

	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = port;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(serverSocket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		closeSocket(serverSocket);
		WSACleanup();
		exit(1);
	}

	std::thread clientConnectionThread([this] { this->acceptNewClients(); });
	clientConnectionThread.detach();

	std::string command;
	while (true) {
		std::cin >> command;
		if (command.compare("q") == 0) {
			break;
		}
	}

	closeAllClients();
}

void Server::closeAllClients() {
	closed = true;
}

void Server::acceptNewClients() {
	SOCKET newsockfd;
	unsigned int clilen;
	struct sockaddr_in cli_addr;
	listen(serverSocket, 5);
	clilen = sizeof(cli_addr);
	while (!closed) {
		if (activeClientsCount == maxClientsCount) {
			continue;
		}
		newsockfd = accept(serverSocket, (struct sockaddr *) &cli_addr, (int*)&clilen);
		if (newsockfd < 0) {
			continue;
		}
		std::thread handleClientConnectionThread([this, newsockfd] { this->handleClientConnection(newsockfd); });
		handleClientConnectionThread.detach();

		ScopeLock lock(activeClientsCountMutex);
		activeClientsCount++;
	}
}

void Server::handleClientConnection(SOCKET newsockfd) {
	struct timeval timeout;
	timeout.tv_sec = readTimeoutInMilliseconds;
	timeout.tv_usec = 0;
	// On timeout read returns -1
	setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

	char buffer[256];
	int totalBytesCount;
	int additionalBytesCount;
	int messageBytesLength;

	messageBytesLength = 0;

	while (!closed) {
		// Start reading new message
		bzero(buffer, sizeof(buffer));

		totalBytesCount = recv(newsockfd, buffer, sizeof(buffer) - 1, 0);

		if (totalBytesCount == -1) {
			std::cout << "omg\r\n";
			continue;
		}
		else if (totalBytesCount <= 0) {
			cleanUpClientConnectionThread(newsockfd);
			return;
		}

		messageBytesLength = buffer[0];

		while (messageBytesLength > totalBytesCount) {
			// Continue reading message to assemble full size message
			if (closed) {
				cleanUpClientConnectionThread(newsockfd);
				return;
			}

			additionalBytesCount = recv(newsockfd, buffer + totalBytesCount, sizeof(buffer) - 1 - totalBytesCount, 0);

			if (additionalBytesCount == -1) {
				continue;
			}
			else if (additionalBytesCount <= 0) {
				cleanUpClientConnectionThread(newsockfd);
				return;
			}

			totalBytesCount += additionalBytesCount;
		}

		/* Write a response to the client */
		bzero(buffer, sizeof(buffer));
		sprintf_s(buffer + 1, sizeof(buffer) - 1, "I got your message");
		buffer[0] = 18;
		totalBytesCount = send(newsockfd, buffer, buffer[0] + 1, 0);

		if (totalBytesCount < 0) {
			cleanUpClientConnectionThread(newsockfd);
			return;
		}
	}

	// Server is finishing -> cleaning up
	cleanUpClientConnectionThread(newsockfd);
}

void Server::cleanUpClientConnectionThread(SOCKET newsockfd) {
	closeSocket(newsockfd);

	ScopeLock lock(activeClientsCountMutex);
	activeClientsCount--;
}

void Server::closeSocket(SOCKET socket) {
	shutdown(socket, SD_BOTH);
	closesocket(socket);
}
