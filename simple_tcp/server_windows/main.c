#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
#include <winsock2.h>

#include <unistd.h>
#include <string.h>


DWORD WINAPI connection_handler(LPVOID);
int main(int argc, char *argv[]) {

	WSADATA wsaData;

    SOCKET sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

	/* Initialize Winsock */
	unsigned int s;
	s = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (s != 0)
    {
        printf("WSAStartup failed: %ui\n", s);
        exit (1);
	}

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
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
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
		closesocket(sockfd);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);


    /* Accept actual connection from the client */
    while (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))
	{
		if(CreateThread(NULL, 0, connection_handler, (void*) &newsockfd, 0, NULL)<0)
		{
            perror("could not create thread");
			closesocket(sockfd);
            return 1;
        }
    }
	if (newsockfd < 0) {
        perror("ERROR on accept");
		closesocket(sockfd);
        exit(1);

	}
    return 0;



}


 DWORD WINAPI connection_handler(LPVOID sockfd)
{
    //Get the socket descriptor
    int sockt = *(int*)sockfd;
    int r_size,n;
    char *new_message , buffer[256];


    //Receive a message from client
    while( (r_size = recv(sockt , buffer , 256 , 0)) > 0 )
    {

        if (r_size < 0)
        {
            perror("ERROR reading from socket");
            closesocket(sockt);
            exit(1);
        }

        printf("Here is the message: %s\n", buffer);

    /* Write a response to the client */


        //end of string marker
		buffer[r_size] = '\0';

		//Send the message back to client
		n = send(sockt, "I got your message", 18, 0); // send on Windows

    	if (n < 0)
		{
        	perror("ERROR writing to socket");
			closesocket(sockt);
        	exit(1);
    	}

		//clear the message buffer
		memset(buffer, 0, 256);
    }

     	if(r_size == -1)
	 	{
        	puts("Client disconnect ");
        	fflush(stdout);
    	}
    	else if(r_size == 0)
    	{
        	perror("Fail recv");
			closesocket(sockt);
        }

    	closesocket(sockt);
    	return 0;
}
