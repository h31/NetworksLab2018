#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

void *connection_handler(void *);

int main(int argc, char *argv[]) {
    int sockfd, newsockfd,*new_socket;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
   
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
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
		pthread_t new_thread;
		new_socket = malloc(sizeof *new_socket);
		*new_socket = newsockfd;
		if( pthread_create( &new_thread , NULL ,  connection_handler , (void*) new_socket) < 0)
        {
            perror("ERROR creating thread");
            return 1;
        }
	}
	
    if (newsockfd < 0) {
        perror("ERROR on accept");
        exit(1);
    }

    return 0;
}
void *connection_handler(void *sockfd)
{
    //Get the socket descriptor
    int sockt = *(int*)sockfd;
    int received_size;
    char buffer[256];

    //Receive a message from client
    while( (received_size = recv(sockt , buffer , 256 , 0)) > 0 )
    {
        //Send the message back to client
        write(sockt , buffer , strlen(buffer));
        printf("Here is the message: %s\n", buffer);
		memset(buffer ,'\0', 256);
    }

    if(received_size == 0)
    {
        puts("Client has been disconnected");
        fflush(stdout);
    }
    else if(received_size == -1)
    {
        perror("ERROR recv");
    }

    //Free and close socket, terminate the socket
    free(sockfd);
    close(sockt);
    pthread_exit(0); 
    return 0;
}
	
