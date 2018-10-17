
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

SOCKET sockfd;
BOOL serverListening = TRUE;

void close_sock(SOCKET sockfd) {
    printf("Server is closing\n");
    shutdown(sockfd, SD_BOTH);
    closesocket(sockfd);
}

void thread_close(SOCKET sockfd){
    printf("Thread closing\r\n");
    closesocket(sockfd);
}

DWORD WINAPI communication(LPVOID args) {

    SOCKET sock, newsock;
    int newsockfd = *(int*) args;
    char buffer[256];
    int c;
    ssize_t n;
    char mesg[256];
    ssize_t realMesgLen;
    ssize_t curMesgLen;

    printf("\nInitialising Winsock...");

    n = recv(newsockfd, buffer, 255, 0); // recv on Windows

    if (n < 0) {
        printf("ERROR reading from socket: %d\n", WSAGetLastError());
        thread_close(newsockfd);
        return GetLastError();
    }

    realMsgLen = buffer[0];
    curMsgLen = n;
    printf("Message size = %u, current size = %u \n", realMsgLen, curMsgLen);
    strcat(msg, buffer + 1);

    n = send(newsockfd, "I got your message", 18, 0); // send on Windows

    thread_close(newsockfd);
    return 0;
}

DWORD WINAPI listening() {
    printf("Listening\n");
    SOCKET newsockfd;
    unsigned int client_len;
    struct sockaddr_in cli_addr;

    if (listen(sockfd, 5) == SOCKET_ERROR) {
        printf( "Listen error:%d\n", WSAGetLastError());
        return GetLastError();
    }

    clilen = sizeof(cli_addr);

    while (serverListening) {

        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (int *)&clilen);

        if (!serverListening) {
            break;
        }

        printf("newsock = %d\n", newsockfd);
        if (newsockfd == INVALID_SOCKET) {
            printf("ERROR on accept: %d\n", WSAGetLastError());
            break;//endThread(sockfd);
        }

        printf("Accept connection\n");

        if (CreateThread(NULL, 0, communicateWithNewClient, &newsockfd, 0, NULL) == NULL) {
            printf("Can't create thread, status = %lu\n", GetLastError());
            //exit(1);
        }
    }

    thread_close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {

    WSADATA wsaData;
    int stat;
    stat = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (stat != 0) {
        printf("WSAStartup failed: %ui\n", stat);
        exit(1);
    }

    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    char *p = buffer;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        WSACleanup();
        exit(1);
    }

    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        close_sock(sockfd);
        WSACleanup();
        exit(1);
    }

    if (CreateThread(NULL, 0, listenForConnection, NULL, 0, NULL) == NULL) {
        printf("Can't create thread, stat = %lu\n", GetLastError());
        close_sock(sockfd);
        WSACleanup();
        exit(1);
    }

    checkKey = 0;

    while (1) {
        printf("Write Q to exit \n\n");
        if ((checkKey = getchar()) == 'q') {
            serverListening = FALSE;
            break;
        }
    }

    close_sock(newsockfd);
    WSACleanup();
    return 0;
}