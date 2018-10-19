#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include<pthread.h>
#include <string.h>
void *connection_handler(void *);
int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
    {
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
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
	close(sockfd);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    pthread_t thread_num;

    /* Accept actual connection from the client */
    while (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen))
    {
		if( pthread_create( &thread_num , NULL , connection_handler , (void*) &newsockfd) < 0)
		{
            		perror("could not create thread");
			close(newsockfd);
            		return 1;
        	}
    }
	if (newsockfd < 0) 
	{
        	perror("ERROR on accept");
		close(sockfd);
        	exit(1);

    		return 0;
	}
}
void *connection_handler(void *sockfd)
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
		close(sockt);
		pthread_exit(NULL);
        	exit(1);
    	}

    	printf("Here is the message: %s\n", buffer);

    	/* Write a response to the client */
	

        //end of string marker
	buffer[r_size] = '\0';
		
	//Send the message back to client
	n=write(sockt, "I got your message", 18); // send on Windows

    	if (n <= 0) 
	{
        	perror("ERROR writing to socket");
		close(sockt);
		pthread_exit(NULL);
        	exit(1);
    	}
		
	//clear the message buffer
	memset(buffer, 0, 256);
    shutdown(sockt, SHUT_RDWR);
	close(sockt);
	pthread_exit(NULL);
    }
     	if(r_size == 0)
	{	
        	puts("Client disconnect ");
        	fflush(stdout);
    	}
    	else if(r_size == -1)
    	{
        	perror("Fail recv");
   	}

    	free(sockfd);
    shutdown(sockt, SHUT_RDWR);
    	close(sockt);
	pthread_exit(NULL);
    	return 0;
} 
