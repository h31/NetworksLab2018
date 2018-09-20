#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <pthread.h>
#include <signal.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

int sockfd;

int const MAX_CLIENT_COUNT = 10;
int totalActiveClientsCount = 0;
int isFinishing = FALSE;

pthread_mutex_t mutex;

void closeSocket(int socket) {
	printf("Closing socket = %d \r\n", socket);
	shutdown(socket, SHUT_RDWR);
	close(socket);
}

void cleanUpClientConnectionThread(int newsockfd) {
	closeSocket(newsockfd);
	
	pthread_mutex_lock(&mutex);
	printf("Handler starts deactivating for newsockfd = %d\r\n", newsockfd);
	totalActiveClientsCount--;
	printf("Total active clients = %d\r\n", totalActiveClientsCount);
    pthread_mutex_unlock(&mutex);
	
	pthread_exit(0);
}

void* handleClientConnection(void* arg) {
	int newsockfd = (int*) arg;
	
    char buffer[256];
	ssize_t n;
	
	bzero(buffer, 256);
	
	while (!isFinishing) {
		n = read(newsockfd, buffer, 255);

		if (n < 0) {
			perror("ERROR reading from socket");
			continue;
		}

		printf("Here is the message: %s\n", buffer);

		/* Write a response to the client */
		n = write(newsockfd, "I got your message", 18);

		if (n < 0) {
			perror("ERROR writing to socket");
			cleanUpClientConnectionThread(newsockfd);
			return;
		}
	}
	
	// Server is finishing -> cleaning up
	cleanUpClientConnectionThread(newsockfd);
}

void* acceptNewClients(void* arg) {
	printf("Ready for accepting incomming connections, sockfd = %d\r\n", sockfd);

	int newsockfd;
    unsigned int clilen;
    struct sockaddr_in cli_addr;

	listen(sockfd, 5);
    clilen = sizeof(cli_addr);

	pthread_t nextClientThread;
	
	while (TRUE) {
		if (isFinishing) {
			// Server finishing
			return;
		}
		if (totalActiveClientsCount == MAX_CLIENT_COUNT) {
			// Limit for maximum clients count
			continue;
		}
		
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("ERROR ON ACCEPT, SKIPPING");
			continue;
		}
		
		pthread_mutex_lock(&mutex);
		totalActiveClientsCount++;
		printf("Creating handler for newsockfd = %d\r\n", newsockfd);
		printf("Total active clients = %d\r\n", totalActiveClientsCount);
		pthread_mutex_unlock(&mutex);
		
		pthread_create(&nextClientThread, NULL, handleClientConnection, (void*) newsockfd);
	}
}

void mainThreadCleanUp() {
	isFinishing = TRUE;
	
	printf("Started cleaning up \r\n");
	
	while (totalActiveClientsCount != 0) {
		// Wait...
		printf("There are some clients on the end, count = %d\r\n", totalActiveClientsCount);
		sleep(1);
	}
	
	printf("Deactivating server socket \r\n");
	closeSocket(sockfd);
	
	printf("Program finished\r\n");
	
	exit(0);
}

int main() {
	// When CTRL+C pressed within terminal -> cleaning up
	signal(SIGINT, mainThreadCleanUp);
	
	struct sockaddr_in serv_addr;
	uint16_t portno;

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
	
	// Creating second thread for handling incoming connections
	pthread_t incomingConnectionsWorkerThread;
	int workerThreadCreationResult;
	workerThreadCreationResult = pthread_create(&incomingConnectionsWorkerThread, NULL, acceptNewClients, (void*) sockfd);
	if (workerThreadCreationResult != 0) {
		printf("Error on creating worker thread, error number: %d\r\nFinishing", workerThreadCreationResult);
	}
	
    // Reading server-side commands from terminal
	char command; 
	
	while (TRUE) {
		command = getchar();
		if (command == 'q') {
			break;
		} else {
			// Do some stuff
		}
	}
	
	// Perfect moment for cleaning up
	mainThreadCleanUp();
		
    return 0;
}
