#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include <pthread.h>

#pragma pack(push, 1);

typedef struct Data_s{
	char dataSize;
	char data[256];
} Data;

#pragma pack(pop);

void *connection_handler(void *);

int main(int argc, char *argv[]) {
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

    if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), & (int) {1}, sizeof(int)) < 0) {
    	perror("ERROR on setsockopt");
    	close(sockfd);
        shutdown(sockfd, 2);
	exit(1);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
	close(sockfd);
        shutdown(sockfd, 2);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    if(listen(sockfd, 5) < 0)
    {
	perror("listen");
	close(sockfd);
        shutdown(sockfd, 2);
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
	    close(sockfd);
    	    shutdown(sockfd, 2);
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

    close(sockfd);
    shutdown(sockfd, 2);
    exit(0);
}

void *connection_handler(void *socket_desc)
{
    char recvbuf[sizeof(Data)*2];
    int recvbuflen = 0;
    Data * bufdata = (Data *) recvbuf;
    int sock = *(int*)socket_desc;
    char* buffer = 0;
    ssize_t n, bufferSize;

    for(;;){
        n = recv(sock, recvbuf + recvbuflen, sizeof(Data), NULL); 
        if (n < 0) {
	    perror("ERROR reading from socket");
	    close(sock);
	    shutdown(sock, 2);
	    pthread_exit(1);
        }

        if (n == 0) {
	    perror("Connection closed");
	    fflush(stdout);
	    close(sock);
	    shutdown(sock, 2);
	    pthread_exit(2);
        }
	
	recvbuflen+=n;	

	if (bufdata->dataSize == 0) {
	    buffer = realloc(buffer, bufferSize + 1);
            buffer[bufferSize] = 0;
            printf("Here is the message: %s\n", buffer);
	    fflush(stdout);
            free(buffer);
	}

	else {

	    if (buffer == 0){
		buffer = malloc(bufdata->dataSize);
		memcpy(buffer, bufdata->data, bufdata->dataSize);
		bufferSize = bufdata->dataSize;
	    }

	    else{
		buffer = realloc(buffer, bufferSize + bufdata->dataSize);
		memcpy(buffer + bufferSize, bufdata->data, bufdata->dataSize);
		bufferSize += bufdata->dataSize;
	    }

	    memcpy(recvbuf, recvbuf + bufdata->dataSize + 1, bufdata->dataSize + 1);
	    recvbuflen -= bufdata->dataSize + 1;
	}
    }
    close(sock);
    shutdown(sock, 2);
    pthread_exit(0);
}
