#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <winsock2.h>

#include <unistd.h>
#include <string.h>

#define bzero(b, len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1, b2, len) (memmove((b2), (b1), (len)), (void) 0)

SOCKET sockfd;
BOOL serverListening = TRUE;

void closeSocket(SOCKET sockfd) {
    printf("Close socket\n\n");
    shutdown(sockfd, SD_BOTH);
    closesocket(sockfd);
}

void endThread(SOCKET sockfd) {
    printf("Thread exit\n");
    closeSocket(sockfd);
}

DWORD WINAPI communicateWithNewClient(LPVOID args) {
    /* If connection is established then start communicating */
    printf("Communication with client starts\n");

    int newsockfd = *(int*) args;
    char buffer[256];
    ssize_t realMsgLen;
    ssize_t curMsgLen;
    char msg[256];
    ssize_t n;

    bzero(buffer, 256);
    bzero(msg, 256);

    n = recv(newsockfd, buffer, 255, 0); // recv on Windows

    if (n < 0) {
        printf("ERROR reading from socket: %d\n", WSAGetLastError());
        endThread(newsockfd);
        return GetLastError();
    }

    realMsgLen = buffer[0];
    curMsgLen = n;
    printf("Message size = %u, current size = %u \n", realMsgLen, curMsgLen);
    strcat(msg, buffer + 1);

    while(realMsgLen > curMsgLen) {
        n = recv(newsockfd, buffer + curMsgLen, realMsgLen - curMsgLen, 0); // recv on Windows
        if (n < 0) {
            printf("ERROR reading from socket: %d\n", WSAGetLastError());
            endThread(newsockfd);
            return GetLastError();
        }
        strcat(msg, buffer + curMsgLen);
        curMsgLen += n;
        printf("Message size = %u, current size = %u \n", realMsgLen, curMsgLen);
        //strcat(msg, buffer + n);
    }

    printf("Here is the message: %s\n", msg);

    /* Write a response to the client */
    n = send(newsockfd, "I got your message", 18, 0); // send on Windows

    if (n < 0) {
        printf("ERROR writing to socket: %d\n", WSAGetLastError());
        endThread(newsockfd);
        return GetLastError();
    }

    endThread(newsockfd);
    return 0;
}

DWORD WINAPI listenForConnection() {
    printf("Listening\n");
    SOCKET newsockfd;
    //int status;
    unsigned int clilen;
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

    endThread(sockfd);
    return 0;
}

int main() {
    WSADATA wsaData;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    int status;
    int checkKey;

    /* Initialize Winsock */
    status = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (status != 0) {
        printf("WSAStartup failed with error: %u\n", status);
        exit(1);
    }

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("ERROR opening socket: %d\n", WSAGetLastError());
        WSACleanup();
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("ERROR on binding: %d\n", WSAGetLastError());
        closeSocket(sockfd);
        WSACleanup();
        exit(1);
    }

    if (CreateThread(NULL, 0, listenForConnection, NULL, 0, NULL) == NULL) {
        printf("Can't create thread, status = %lu\n", GetLastError());
        closeSocket(sockfd);
        WSACleanup();
        exit(1);
    }

    checkKey = 0;

    while (1) {
        printf(">>>>>>>>WRITE q TO EXIT<<<<<<<<\n\n");
        if ((checkKey = getchar()) == 'q') {
            serverListening = FALSE;
            break;
        }
    }

    closeSocket(sockfd);
    WSACleanup();
    printf("\nBYE.\n");
    return 0;
}

