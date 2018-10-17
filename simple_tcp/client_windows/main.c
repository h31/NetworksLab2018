#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#pragma comment (lib, "Ws2_32.lib")

int main(int argc, char *argv[]) {
	int n;
	SOCKET sockfd;
	unsigned int portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	WSADATA wsa;

	char buffer[256];

	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = (unsigned int)atoi(argv[2]);

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(1);
	}
	printf("Initialised.\n");

	/* Create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == NULL) {
		perror("ERROR opening socket");
		exit(1);
	}

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length); 
	serv_addr.sin_port = htons(portno);

	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		perror("ERROR connecting");
		closesocket(sockfd);
		exit(1);
	}

	/* Now ask for a message from the user, this message
	* will be read by server
	*/
	printf("Please enter the message: ");
	memset(buffer,0,  256);
	fgets(buffer, 255, stdin);

	int message_length = strlen(buffer);

	/* Send message length to the server */
	n = send(sockfd, (char*)&message_length, sizeof(int), 0);

	if (n < 0) {
		perror("ERROR writing length to socket");
		exit(1);
	}

	/* Send message to the server */
	n = send(sockfd, buffer, strlen(buffer), 0);

	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	/* Now read server response */
	memset(buffer,0, 256);
	n = recv(sockfd, buffer, 255, 0);

	if (n < 0) {
		perror("ERROR reading from socket");
		exit(1);
	}

	printf("%s\n", buffer);
	exit(0);
}