#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
int check_1 = 1;
void *connection_handler(void *);

int main(int argc, char *argv[]) 
{
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
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

     if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), & (int) {1}, sizeof(int)) < 0) 
    {
    	perror("ERROR on setsockopt");
    	shutdown(sockfd, 2);
    	close(sockfd);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
	shutdown(sockfd, 2);
    	close(sockfd);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    if(listen(sockfd, 5) < 0)
    {
	perror("listen");
	exit(EXIT_FAILURE);
    }

    puts("Waiting for incoming connections");
    clilen = sizeof(cli_addr);

    while( (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)))
    {

	puts("Connection accepted");

	pthread_t sn_thread;

	if( pthread_create( &sn_thread, NULL, connection_handler, &newsockfd) < 0)
	{
	    perror("Could not create thread");
	    return 1;
	}

	pthread_detach(sn_thread);
	puts("Handler assigned");
    }

    if (sockfd < 0)
    {
	perror("accept failed");
	return 1;
    }

    shutdown(sockfd, 2);
    close(sockfd);
    exit(0);
}

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char dataSize;
    char dataBuffer[256];
    int buffer_1_length = 0;
    char buffer_1 [sizeof(dataBuffer)*2];
    char* buffer = 0;
    ssize_t n, bufferSize;

    for(;;)
    {
        n = recv(sock, buffer_1 + buffer_1_length, sizeof(dataBuffer), NULL); 
        if (n < 0) 
        {
	    perror("ERROR reading from socket");
	    close(sock);
	    shutdown(sock, 2);
	    pthread_exit(1);
        }

        if (n == 0) 
        {
	    perror("Connection closed");
	    fflush(stdout);
	    close(sock);
	    shutdown(sock, 2);
	    pthread_exit(2);
        }
	
	buffer_1_length+=n;	

	if (dataSize == 0) 
        {
	    buffer = realloc(buffer, bufferSize + 1);
            buffer[bufferSize] = 0;
            if (check_1 ==1)
	{
            printf("Here is the message: %s\n", buffer_1);
	    check_1 = -check_1;
	   
	}
	else check_1 = -check_1;
	}
	else 
        {

	    if (buffer == 0)
            {
		buffer = malloc(dataSize);
		memcpy(buffer, dataBuffer, dataSize);
		bufferSize = dataSize;
	    }

	    else
            {
		buffer = realloc(buffer, bufferSize + dataSize);
		memcpy(buffer + bufferSize, dataBuffer, dataSize);
		bufferSize += dataSize;
	    }

	    memcpy(buffer_1, buffer_1 + dataSize + 1, dataSize + 1);
	    buffer_1_length -= dataSize + 1;
	}
    }
    shutdown(sock, 2);
    close(sock);
    pthread_exit(0);
}
