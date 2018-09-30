#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>

#include <string.h>
#define SHUT_RDWR 2

int main(int argc, char *argv[]) {
    SOCKET sockfd;
	int n;
    long messlen;
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    char bufforlen[sizeof(long)*8+1];

	WSADATA WSStartData; 

	if (WSAStartup(MAKEWORD(2, 2), &WSStartData) != 0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (int) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);

        exit(0);
    }

    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length); 
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        perror("ERROR connecting");
		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);

        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    messlen = strlen(buffer);

    sprintf(bufforlen, "%d", messlen);

    n = send(sockfd, bufforlen, strlen(bufforlen), 0);

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        shutdown(sockfd, SHUT_RDWR);
        closesocket(sockfd);

        exit(1);
    }

    n = send(sockfd, buffer, messlen, 0);

    if (n < 0) {
        perror("ERROR writing to socket message");
		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);

        exit(1);
    }

    /* Now read server response */
    memset(buffer, 0, 256);
    n = recv(sockfd, buffer, 255, 0);

    if (n < 0) {
        perror("ERROR reading from socket");
		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);

        exit(1);
    }

    printf("%s\n", buffer);

	shutdown(sockfd, SHUT_RDWR);
	closesocket(sockfd);

    return 0;
}
