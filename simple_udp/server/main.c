#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    ssize_t n;
    n = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (n != 0) {
        printf("WSAStartup failed: %zi\n", n);
        return 1;
    }

    int i;
    uint16_t portno;
    int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */
    i = (int) socket(AF_INET, SOCK_DGRAM, 0);

    if (i < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind((SOCKET) i, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    clilen = sizeof(cli_addr);
    memset(buffer, 0, 256);
    n = recvfrom((SOCKET) i, buffer, 255, 0, (struct sockaddr*)&cli_addr, &clilen);

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Here is the message: %s\n", buffer);

    /* Write a response to the client */
    n = sendto((SOCKET) i, "I got your message", 18, 0,
               (struct sockaddr*) &cli_addr, clilen);

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

    /* Closing socket */
    n = shutdown((SOCKET) i, SD_BOTH);
    if (n == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket((SOCKET) i);
        WSACleanup();
        return 1;
    }
    closesocket((SOCKET) i);
    WSACleanup();

    return 0;
}