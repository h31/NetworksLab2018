
#include <string.h>
#include <winsock2.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int n;
    WSADATA wsaData;
    SOCKET sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    char msg[256];
    int msgsize;
    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    n = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (n != 0) {
        printf("WSAStartup failed: %d\n", n);
        exit(1);
    }

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
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
    
    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR connecting");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255, stdin);
    msgSize = strlen(buffer);
    msg[0] = msgSize;
    strcat(msg, buffer);

    /* Send message to the server */
    n = send(sockfd, buffer, strlen(buffer), 0);

    if (n < 0) {
        printf("ERROR writing to socket");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    /* Now read server response */

    memset(buffer, 0, 256);
    n = recv(sockfd, buffer, 255, 0);

    if (n < 0) {
        printf("ERROR reading from socket");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    printf("%s\n", buffer);
    closesocket(sockfd);
    WSACleanup();
    
    /* Closing socket */
    n = shutdown(sockfd, SD_BOTH);
    if (n == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }
    closesocket(sockfd);
    WSACleanup();
    
    return 0;
}