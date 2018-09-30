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

int const MAX_SECONDS_READ_TIMEOUT = 5;

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

void handleClientConnection(void* arg) {
	int newsockfd = (int*) arg;
	
	struct timeval timeout;
	timeout.tv_sec = MAX_SECONDS_READ_TIMEOUT;
	timeout.tv_usec = 0;
	// On timeout read returns -1
	setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
	
	char buffer[256];
	ssize_t totalBytesCount;
	ssize_t additionalBytesCount;
	ssize_t messageBytesLength;
	
	messageBytesLength = 0;
	
	while (!isFinishing) {
		// Start reading new message
		bzero(buffer, sizeof(buffer));
		
		totalBytesCount = read(newsockfd, buffer, sizeof(buffer) - 1);
		
		if (totalBytesCount == -1) {
			continue;
		} else if (totalBytesCount <= 0) {
			cleanUpClientConnectionThread(newsockfd);
		}
		
		messageBytesLength = buffer[0];
		
		printf("Initially read [%d] bytes of [%d] messageBytesLength\r\n", totalBytesCount, messageBytesLength);
		
		while (messageBytesLength > totalBytesCount) {
			// Continue reading message to assemble full size message
			printf("Reading additional bytes from = %d\r\n", newsockfd);
			if (isFinishing) {
				cleanUpClientConnectionThread(newsockfd);
			}
			
			additionalBytesCount = read(newsockfd, buffer + totalBytesCount, sizeof(buffer) - 1 - totalBytesCount);
			
			if (additionalBytesCount == -1) {
				continue;
			} else if (additionalBytesCount <= 0) {
				cleanUpClientConnectionThread(newsockfd);
			}

			totalBytesCount = totalBytesCount + additionalBytesCount;
			
			printf("Read [%d] bytes of [%d] messageBytesLength\r\n", totalBytesCount, messageBytesLength);
		}
		
		printf("Read from = %d:\r\n%s", newsockfd, buffer + 1);

		/* Write a response to the client */
		totalBytesCount = write(newsockfd, "I got your message", 18);

		if (totalBytesCount < 0) {
			printf("ERROR ON SENDING BACK, DISCONNECTING\r\n");
			cleanUpClientConnectionThread(newsockfd);
		}
	}
	
	// Server is finishing -> cleaning up
	cleanUpClientConnectionThread(newsockfd);
}

void acceptNewClients() {
	printf("Ready for accepting incomming connections, sockfd = %d\r\n", sockfd);

	int newsockfd;
	unsigned int clilen;
	struct sockaddr_in cli_addr;

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	pthread_t nextClientThread;
	int nextClientThreadCreationResult;
	
	while (!isFinishing) {
		if (totalActiveClientsCount == MAX_CLIENT_COUNT) {
			// Limit for maximum clients count, the printf below can produce infinite flood
			// printf("Connection limit reached, new user will hanging on connect\r\n");
			continue;
		}
		
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			printf("ERROR ON ACCEPT, SKIPPING\r\n");
			continue;
		}
		
		nextClientThreadCreationResult = pthread_create(&nextClientThread, NULL, handleClientConnection, (void*) newsockfd);
		if (nextClientThreadCreationResult != 0) {
			printf("Error on creating client thread, error number: %d\r\n", nextClientThreadCreationResult);
		} else {
			pthread_mutex_lock(&mutex);
			totalActiveClientsCount++;
			printf("Creating handler for newsockfd = %d\r\n", newsockfd);
			printf("Total active clients = %d\r\n", totalActiveClientsCount);
			pthread_mutex_unlock(&mutex);
		}
	}
	
	pthread_exit(0);
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
	workerThreadCreationResult = pthread_create(&incomingConnectionsWorkerThread, NULL, acceptNewClients, NULL);
	if (workerThreadCreationResult != 0) {
		printf("Error on creating worker thread, error number: %d\r\n", workerThreadCreationResult);
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
