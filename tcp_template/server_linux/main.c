#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

/* function for thread */
void *communicate_func (void *arg);

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
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

    /* Accept connections forever */
    while(1) {
    	/* Accept actual connection from the client */
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    	if (newsockfd < 0) {
	    perror("ERROR on accept");
            exit(1);
    	}

	/* Making new thread for messaging with client */
	pthread_t thread;	//thread
	int result;		//result of thread creating
	result = pthread_create(&thread, NULL, communicate_func, &newsockfd); //create new thread
	if(result != 0) {
	    perror("Error while creating thread");
	}
    }
    close(sockfd);
    exit(0);
}

/* waiting for connection */
void *waiting_func (void *arg) {

}

/* function for thread */
void *communicate_func (void *arg) {
    int newsockfd = * (int *) arg;;
    char buffer[256];
    ssize_t n;

    /* If connection is established then start communicating */
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255); // recv on Windows

    if (n < 0) {
        perror("ERROR reading from socket");
        pthread_exit(1);
    }

    printf("Here is the message: %s\n", buffer);

    /* Write a response to the client */
    n = write(newsockfd, "I got your message", 18); // send on Windows

    if (n < 0) {
        perror("ERROR writing to socket");
	pthread_exit(1);
    }

    close(newsockfd); //close socket after messaging
    pthread_exit(0);
}
