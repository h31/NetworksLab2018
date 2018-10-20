#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <pthread.h>

#include<arpa/inet.h>

int sockfd;
struct sockaddr_in cli_addr;
unsigned int clilen;

// This thread computes an answer for a client
void *handleRequestThread() {
	int n;
	struct sockaddr_in cli_addr_local = cli_addr;	// Or else the response
													// goes to any last client

	printf("Computing for the client: %s. Client port: %d.\n", inet_ntoa(cli_addr_local.sin_addr), ntohs(cli_addr.sin_port));
	sleep(5); // Operation imitation
	printf("Done.\n\n");

	/* Write a response to the client */
	n = sendto(sockfd, "I got your message", 18, 0, (struct sockaddr*) &cli_addr_local, clilen); // sendto

	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	return 0;
}

void handleRequest() {
	int status;
	pthread_t aThread;

	status = pthread_create(&aThread, NULL, handleRequestThread, NULL);
	if (status != 0) {
		printf("Cant create a thread. Status: %d\n", status);
		exit(2);
	}

	// Shouldnt wait for the end of the thread
}

void *controling() {
	char ch;

	do {
		ch = getchar();
	} while (ch != 'q');

	printf("Exiting...\n");
	close(sockfd);
	printf("The server is off now\n\n");
	exit (0);

	// return 0;
}

int main(int argc, char *argv[]) {
	uint16_t portno;
	struct sockaddr_in serv_addr;
	ssize_t n;
	char buffer[256];

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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

	//listen(sockfd, 5);
	clilen = sizeof(cli_addr);


	while(1) {
		/* Accept messages from the clients */
		bzero(buffer, 256);
	
		//while(1) {
			n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr *) &cli_addr, &clilen); // recvfrom
			printf("Here is the message/part of the message: %s", buffer);
		//	bzero(buffer, 256);
		//	if (n < 255) break;
		//}
		//printf("---End of the input data---\n");

		handleRequest();
	}
}
