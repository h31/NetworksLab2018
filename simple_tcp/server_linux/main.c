#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <pthread.h>

int sockfd;
#define max_clients 10
#define true 1
#define false 0
int cliPorts[max_clients];
int isActive[max_clients];	// bool

typedef struct {
	int socket;
	// May be extended later
} requestDataStruct;

int addNewPortToList(int port) {
	int i = 0;
	for (; i < max_clients; i = i + 1) {
		if (isActive[i] == false) {
			isActive[i] = true;
			cliPorts[i] = port;
			return i;
		}
	}
	printf("Cant create a new connection!");
	exit(3);
}

void closeClientConnections() {
	int i = 0;
	for (; i < max_clients; i = i + 1) {
		if (isActive[i] == true) {
			shutdown(cliPorts[i], 2);
			close(cliPorts[i]);
		}
	}
	printf("Client connections have been closed...\n");
}

void *handleRequestThread(void *requestData) {
	char buffer[256];
	int newsockfd, n, boolplace;

	requestDataStruct *rds = (requestDataStruct*) requestData;

	newsockfd = rds->socket;
	boolplace = addNewPortToList(newsockfd);

	/* If connection is established then start communicating */
    bzero(buffer, 256);
	
	while(1) {
		n = recv(newsockfd, buffer, 255, 0); // recv on Linux
		printf("Here is the message/part of the message: %s\n", buffer);
		bzero(buffer, 256);
		if (n < 255) break;
	}
	printf("---End of the input data---\n");

	sleep(5); // Operation imitation

    /* Write a response to the client */
    n = send(newsockfd, "I got your message", 18, 0); // send on Linux

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }

	shutdown(newsockfd, 2);
	close(newsockfd);
	isActive[boolplace] = false;

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

void *controling(void *args) {
	char ch;

	do {
		ch = getchar();
	} while (ch != 'q');

	printf("Exiting...\n");
	shutdown(sockfd, 2);
	close(sockfd);

	return 0;
}

int main(int argc, char *argv[]) {
    int newsockfd;
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

	/* Creating a control thread */
	int status;
	pthread_t controlThread;

	status = pthread_create(&controlThread, NULL, controling, NULL);
	if (status != 0) {
		printf("Cant create a control thread. Status: %d\n", status);
		exit(2);
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
            break;
        }

		handleRequest(newsockfd);
    }

	closeClientConnections();
	printf("The server is off now\n\n");
	exit (0);
}
