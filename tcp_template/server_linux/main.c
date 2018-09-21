#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>


void* connection_handler(void* arg) {

    int newsockfd = (int*) arg;
    ssize_t n; 
    char buffer[256];

/* If connection is established then start communicating */
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255); // recv on Windows

    if (n < 0) {
        perror("ERROR reading from socket");  
	exit(0);
    }

    printf("Here is the message: %s\n", buffer);

    /* Write a response to the client */

    if (n <= 0) {
        perror("ERROR writing to socket");
	shutdown(newsockfd, SHUT_RDWR);
	close(newsockfd);	       
	pthread_exit(0);
    }
    shutdown(newsockfd, SHUT_RDWR);
    close(newsockfd);	       
    pthread_exit(0); 
}


int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    pthread_t thread;
    int created_thread;

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

    while(1) {  
    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
        perror("ERROR on accept");       
	exit(1);
    }

    if(created_thread = pthread_create(&thread, NULL, connection_handler, (void*) newsockfd) <= 0)
    {
    perror("ERROR creating thread");
    }

    //pthread_exit(0);
    }

    //shutdown(sockfd, SHUT_RDWR),
    //close(sockfd),
    exit(0);
    return 0;
}
