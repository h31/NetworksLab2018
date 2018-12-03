#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>

#include <unistd.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    SOCKET sockfd;
    WSADATA wsdt;
    int ws;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);
    ws = WSAStartup(MAKEWORD(2,2), &wsdt);
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        closesocket(sockfd);
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

    memset((char *) &serv_addr,'\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove( (char *) &serv_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
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
    	memset(buffer,'\0', 256);
		fgets(buffer, 255, stdin);

        /* Send message to the server */
        if( send(sockfd , buffer , 256 , 0) < 0)
        {
            puts("ERROR sending");
            return 1;
        }
		/* Now read server response */
		if( recv(sockfd , buffer , 256 , 0) < 0)
        {
            puts("ERROR recv");
            break;
        }

        }
		shutdown(sockfd, SD_BOTH);
    	closesocket(sockfd);
    	WSACleanup();
    	return 0;
}