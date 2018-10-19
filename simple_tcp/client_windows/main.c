#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <winsock2.h>

#include <unistd.h>

#include <string.h>

int main(int argc, char *argv[]) {

	WSADATA wsaData;
    SOCKET sockfd;
    int n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

	unsigned int s;

    s = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (s != 0)
    {
        printf("WSAStartup failed: %ui\n", s);
        return 1;
    }

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        WSACleanup();
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        closesocket(sockfd);
        WSACleanup();
        exit(0);
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        perror("ERROR connecting");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */
	while(1)
	{
    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    n = send(sockfd, buffer, strlen(buffer), 0);

    if (n < 0)
    {
        perror("ERROR writing to socket");
        WSACleanup();
        exit(1);
    }

    /* Now read server response */
    memset(buffer, 0, 256);
    n = recv(sockfd, buffer, 255,0);

    if (n < 0)
    {
        perror("ERROR reading from socket");
        closesocket(sockfd);
        exit(1);
    }

    printf("%s\n", buffer);


	}

	closesocket(sockfd);
	WSACleanup();
    return 0;
}
