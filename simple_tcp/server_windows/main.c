#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <windows.h>
#include <winsock2.h>

#include <string.h>

#define SHUT_RDWR 2

void closeSocket(SOCKET socks[], int lenght, int error, char* errorMsg);
char* readAll(SOCKET socks[]);
void sendAll(SOCKET socks[], char* buffer);
DWORD WINAPI mainLoop(); 
DWORD WINAPI clientCommunication(LPVOID data);

volatile BOOL readyToAccept = TRUE;

int main() {
	WSADATA WSStartData; /* —труктура данных библиотеки сокета. */

	/* »нициализировать библиотеку WSA; задана верси€ 2.2 */
	if (WSAStartup(MAKEWORD(2, 2), &WSStartData) != 0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}

	CreateThread(NULL, 0, mainLoop, NULL, 0, NULL);

	while(getchar() != 'q'){
	}

    return 0;
}

void closeSocket(SOCKET socks[], int lenght, int error, char* errorMsg){
	if(strcmp(errorMsg,"") != 0){
		printf(strcat(errorMsg, " with code : %ld\n"), WSAGetLastError());
	}
	for(int i = 0; i < lenght; i++) {
		shutdown(socks[i], SHUT_RDWR);
		closesocket(socks[i]);
	}
	WSACleanup();
	exit(error);
}

char* readAll(SOCKET socks[]){
	char *buffer = (char*)malloc(256);
	char strLenght[4];

	int n = recv(socks[1], strLenght, 4, 0);
	if (n < 0) {
		closeSocket(socks, 2, 1, "ERROR reading header from socket");
	}
	int lenght = ((strLenght[0] - '0') << 24) + ((strLenght[1] - '0') << 16) + ((strLenght[2] - '0') << 8) + (strLenght[3] - '0');

	memset(buffer, 0, 256);
	int recieved = 0;
	while(recieved < lenght){
		n = recv(socks[1], buffer, 256, 0);
		recieved += n;
		if (n < 0) {
			closeSocket(socks, 2, 1, "ERROR reading from socket");
		}
	}

	return buffer;
}

void sendAll(SOCKET socks[], char* buffer){
    int messageLength = strlen(buffer);
    char toSend[4 + 256];
	memset(toSend, 0, 4 + 255);
    toSend[0] = ((messageLength >> 24) & 0xff) + '0';
    toSend[1] = ((messageLength >> 16) & 0xff) + '0';
    toSend[2] = ((messageLength >> 8) & 0xff) + '0';
    toSend[3] = ((messageLength >> 0) & 0xff) + '0';

    strcat(toSend, buffer);

    int n = send(socks[1], toSend, strlen(toSend), 0);
    if (n < 0) {
        closeSocket(socks, 2, 1, "ERROR writing to socket");
    }
}

DWORD WINAPI mainLoop() {
	SOCKET sockfd;
	unsigned int portno;
	struct sockaddr_in serv_addr;

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == INVALID_SOCKET) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Initialize socket structure */
	memset((char *)&serv_addr, 0, sizeof(serv_addr)); //bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = 5001;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	BOOL value = TRUE;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &value, sizeof(BOOL)) < 0) {
		closeSocket((SOCKET[]) {sockfd}, 1, 1, "ERROR on setsockopt");
	}

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		closeSocket((SOCKET[]) { sockfd }, 1, 1, "ERROR on binding");
	}

	/* Now start listening for the clients, here process will
	* go in sleep mode and will wait for the incoming connection
	*/

	listen(sockfd, 5);

	while (1) {
		if (readyToAccept == TRUE) {
			readyToAccept = FALSE;
			CreateThread(NULL, 0, clientCommunication, &sockfd, 0, NULL);
		}
	}
}

DWORD WINAPI clientCommunication(LPVOID data) {
	int sockfd = *(int *)data;
	struct sockaddr_in cli_addr;
	int clilen = sizeof(cli_addr);

	/* Accept actual connection from the client */
	SOCKET newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

	if (newsockfd == INVALID_SOCKET) {
		closeSocket((SOCKET[]) { sockfd, newsockfd }, 2, 1, "ERROR on accept");
	}

	readyToAccept = TRUE;
	char* buffer = (char*)malloc(256);

	/* If connection is established then start communicating */
	buffer = readAll((SOCKET[]) { sockfd, newsockfd});
	printf("Here is the message: %s\n", buffer);

	/* Write a response to the client */
	sendAll((SOCKET[]) { sockfd, newsockfd }, "I got your message");

	closesocket(newsockfd);

	return 0;
}
