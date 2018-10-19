#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <winsock2.h>
#include <unistd.h>
#include <string.h>

DWORD WINAPI connection_handler(LPVOID);
int main(int argc, char *argv[  ]) {
    SOCKET sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    int ws;
    WSADATA wsdt;
    HANDLE client_thread;
    struct sockaddr_in serv_addr, cli_addr;
    ws = WSAStartup(MAKEWORD(2,2), &wsdt);
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
	{
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    memset((char *) &serv_addr,'\0', sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    /* Accept actual connection from the client */
    while ((newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) )
	{
        client_thread = CreateThread(NULL, 0, connection_handler, &newsockfd, 0, NULL);
	}
	closesocket(sockfd);

    if (newsockfd < 0) 
	{
        perror("ERROR on accept");
        closesocket(sockfd);
        WSACleanup();
        exit(1);
    }

    return 0;
}
DWORD WINAPI connection_handler(LPVOID sockfd)
{
    //Get the socket descriptor
    int sockt = *(int*)sockfd;
    int received_size;
    char buffer[256];

    //Receive a message from client
    while( (received_size = recv(sockt , buffer , 256 , 0))>0)
    {
        //Send the message back to client
        send(sockt , buffer , 256,'\0');
        printf("Here is the message: %s\n", buffer);
		memset(buffer ,'\0', 256);
    }
    if(received_size == -1)
    {
        printf("Client has been disconnected \n");
        fflush(stdout);
        closesocket(sockt);
        return GetLastError();
    }
    else if (received_size == 0)
    {
        printf("ERROR recv");
        closesocket(sockt);
        return GetLastError();
    }
    //Free and close socket
    shutdown(sockt,SD_BOTH);
    closesocket(sockt);
    return 0;
}

