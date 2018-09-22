#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <pthread.h>

typedef struct {
	int socket;
	// May be extended later
} requestDataStruct;

void *handleRequestThread(void *requestData) {
	char buffer[256];
	int newsockfd, n;

	requestDataStruct *rds = (requestDataStruct*) requestData;

	newsockfd = rds->socket;

	/* If connection is established then start communicating */
    bzero(buffer, 256);
    n = recv(newsockfd, buffer, 255, 0); // recv on Linux

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Here is the message: %s\n", buffer);

	sleep(5); // Operation imitation

    /* Write a response to the client */
    n = send(newsockfd, "I got your message", 18, 0); // send on Linux

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

	return 0;
}

void handleRequest(int socket) {
	requestDataStruct rds;
	int status;
	pthread_t aThread;

	rds.socket = socket;

	status = pthread_create(&aThread, NULL, handleRequestThread, (void*) &rds);
	if (status != 0) {
		printf("Cant create a thread. Status: %d\n", status);
		exit(2);
	}

	// Shouldnt wait for the end of the thread
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

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

		handleRequest(newsockfd);
    }

    return 0;
}
