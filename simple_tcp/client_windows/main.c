#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <unistd.h>

#include <string.h>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)

int main(int argc, char *argv[]) {
    SOCKET sockfd;
    WSADATA wsaData;

    int sres, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    char msg[256];
    int msgSize;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);


    // Initialize Winsock
    sres = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (sres != 0) {
        printf("WSAStartup failed with error: %d\n", sres);
        exit(1);
    }

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("ERROR opening socket: %d\n", WSAGetLastError());
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

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        printf("ERROR connecting: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    bzero(buffer, 256);
    bzero(msg, 256);
    fgets(buffer, 254, stdin);
    msgSize = strlen(buffer);
    msg[0] = msgSize;
    //printf("message - %s, size = %d, b_size = %d\n\n", buffer, msgSize, sizeof(msg));
    strcat(msg, buffer);

    /* Send message to the server */
    n = send(sockfd, msg, strlen(msg), 0);

    if (n < 0) {
        printf("ERROR writing to socket: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    /* Now read server response */
    bzero(buffer, 256);
    n = recv(sockfd, buffer, 255, 0);

    if (n < 0) {
        printf("ERROR reading from socket: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    printf("%s\n", buffer);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}

