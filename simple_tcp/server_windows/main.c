#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

SOCKET sockfd;

void close_sock(SOCKET sockfd) {
    printf("Server is closing\n");
    shutdown(sockfd, SD_BOTH);
    closesocket(sockfd);
}

void thread_close(SOCKET sockfd){
    printf("Thread closing\r\n");
    closesocket(sockfd);
}
DWORD WINAPI connection_handler(LPVOID);

DWORD WINAPI connection_handler(LPVOID sockfd) {
    SOCKET newsockfd;
    int sockt = *(int*)sockfd;
    char buffer[256];
    int c;
    ssize_t n;
    char mesg[256];
    ssize_t realMesgLen;
    ssize_t curMesgLen;

    printf("\nInitialising Winsock...");

    while((n = recv(newsockfd, buffer, 255, 0)) > 0); // recv on Windows
    {
        send(sockt , buffer , 255,'\0');
        printf("Here is the message: %s\n", buffer);
		memset(buffer ,'\0', 255);
    }

    if (n < 0) {
        printf("ERROR reading from socket: %d\n", WSAGetLastError());
        thread_close(newsockfd);
        return GetLastError();
    }

    realMesgLen = buffer[0];
    curMesgLen = n;
    printf("Message size = %u, current size = %u \n", realMesgLen, curMesgLen);
    strcat(mesg, buffer + 1);

    n = send(newsockfd, "I got your message", 18, 0); // send on Windows

    shutdown(sockt,SD_BOTH);
    thread_close(newsockfd);
    return 0;
}

int main(int argc, char *argv[]) {

    WSADATA wsaData;
    HANDLE c_thread;
    int stat;
    stat = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (stat != 0) {
        printf("WSAStartup failed: %ui\n", stat);
        exit(1);
    }

    SOCKET sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    char *p = buffer;
    struct sockaddr_in serv_addr, cli_addr;

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

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) )
	{
        c_thread = CreateThread(NULL, 0, connection_handler, &newsockfd, 0, NULL);
	}
	close_sock(sockfd);

    if (newsockfd < 0) 
	{
        perror("ERROR on accept");
        close_sock(sockfd);
        WSACleanup();
        exit(1);
    }

    close_sock(newsockfd);
    WSACleanup();
    return 0;
}