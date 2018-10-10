#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>					// uint16_t

#include <string.h>

#include <windows.h>
#include <winsock2.h>
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

SOCKET sockfd;

typedef struct {
	SOCKET socket;
	// May be extended later
} requestDataStruct;

DWORD WINAPI handleRequestThread(LPVOID requestData) {
	char buffer[256];
	SOCKET newsockfd;
	int n;

	requestDataStruct rds = *(requestDataStruct*)requestData;

	newsockfd = rds.socket;

	/* If connection is established then start communicating */
	bzero(buffer, 256);

	while (1) {
		n = recv(newsockfd, buffer, 255, 0); // recv on Windows
		printf("Here is the message/part of the message: %s\n", buffer);
		bzero(buffer, 256);
		if (n < 255) break;
	}
	printf("---End of the input data---\n");

	Sleep(5000); // Operation imitation

	/* Write a response to the client */
	n = send(newsockfd, "I got your message", 18, 0); // send on Windows

	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	closesocket(newsockfd);

	return 0;
}

void handleRequest(SOCKET socket) {
	requestDataStruct rds;
	HANDLE handle_thread;

	rds.socket = socket;

	handle_thread = CreateThread(NULL, 0, handleRequestThread, &rds, 0, NULL);

	// Shouldnt wait for the end of the thread
}

DWORD WINAPI controling(LPVOID args) {
	char ch;

	do {
		ch = getchar();
	} while (ch != 'q');

	printf("Exiting...\n");
	closesocket(sockfd);

	return 0;
}

int main(int argc, char *argv[]) {
	WSADATA wsaData;

	unsigned int t;
	t = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (t != 0) {
		printf("WSAStartup failed: %ui\n", t);
		exit(1);
	}

	//typedef uint16_t u_int16_t;

	SOCKET newsockfd;
	uint16_t portno;
	int clilen;
	struct sockaddr_in serv_addr, cli_addr;
	int n;

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == INVALID_SOCKET) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Initialize socket structure */
	bzero((char *)&serv_addr, sizeof(serv_addr));
	portno = 5001;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
		perror("ERROR on binding");
		exit(1);
	}

	/* Creating a control thread */
	HANDLE handle_control;

	handle_control = CreateThread(NULL, 0, controling, NULL, 0, NULL);

	/* Now start listening for the clients, here process will
	   * go in sleep mode and will wait for the incoming connection
	*/

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	while (1) {
		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (SOCKADDR *)&cli_addr, &clilen);

		if (newsockfd < 0) {
			perror("ERROR on accept");
			break;
		}

		handleRequest(newsockfd);
	}

	closesocket(newsockfd);
	WSACleanup();
	printf("The server is off now\n\n");
	exit(0);
}
