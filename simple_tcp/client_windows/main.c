#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <winsock2.h>

#include <string.h>

#define SHUT_RDWR 2

void closeSocket(SOCKET sockfd, int error, char* errorMsg);
void sendAll(SOCKET sockfd, char buffer[]);
char* readAll(SOCKET sockfd);

int main(int argc, char *argv[]) {
    SOCKET sockfd;
    unsigned int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char* buffer = (char*)malloc(256);
	WSADATA WSStartData; /* —труктура данных библиотеки сокета. */

	/* »нициализировать библиотеку WSA; задана верси€ 2.2 */
	if (WSAStartup(MAKEWORD(2, 2), &WSStartData) != 0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (unsigned int) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        closeSocket(sockfd, 0, "ERROR, no such host\n");
    }

	//memset(buffer, 0, 256);
	memset((char *)&serv_addr, 0, sizeof(serv_addr)); //bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length); // bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t)server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        closeSocket(sockfd, 1, "ERROR connecting");
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    sendAll(sockfd, buffer);

    /* Now read server response */
    buffer = readAll(sockfd);

    printf("%s\n", buffer);

    closeSocket(sockfd, 0, "");

    return 0;
}

void closeSocket(SOCKET sockfd, int error, char* errorMsg){
    if(strcmp(errorMsg,"") != 0){
        printf(strcat(errorMsg, " with code : %ld\n"), WSAGetLastError());
    }
    shutdown(sockfd, SHUT_RDWR);
    closesocket(sockfd);
	WSACleanup();
	exit(error);
}

void sendAll(SOCKET sockfd, char buffer[]){
    int messageLength = strlen(buffer);
    char toSend[4 + 256];
	memset(toSend, 0, 4 + 255);
    toSend[0] = ((messageLength >> 24) & 0xff) + '0';
    toSend[1] = ((messageLength >> 16) & 0xff) + '0';
    toSend[2] = ((messageLength >> 8) & 0xff) + '0';
    toSend[3] = ((messageLength >> 0) & 0xff) + '0';

    strcat(toSend, buffer);

    int n = send(sockfd, toSend, strlen(toSend), 0);
    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR writing to socket");
    }
}

char* readAll(SOCKET sockfd){
    char *buffer = (char*)malloc(256);
    char strLenght[4];

    int n = recv(sockfd, strLenght, 4, 0);
    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR reading from socket");
    }
    int lenght = ((strLenght[0] - '0') << 24) + ((strLenght[1] - '0') << 16) + ((strLenght[2] - '0') << 8) + (strLenght[3] - '0');

	memset(buffer, 0, 256);
    int recieved = 0;
    while(recieved < lenght){
        n = recv(sockfd, buffer, 256, 0);
        recieved += n;
        if (n < 0) {
            closeSocket(sockfd, 1, "ERROR reading from socket");
        }
    }

    return buffer;
}