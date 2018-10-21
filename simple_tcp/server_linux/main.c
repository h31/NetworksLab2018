#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../util_linux/util_linux.h"

int main() {
	int sockfd, newsockfd;
	uint16_t portno;
	unsigned int clilen;
	char* buffer;
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

	//https://serverfault.com/questions/329845/how-to-forcibly-close-a-socket-in-time-wait
	if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &(int){ 1 }, sizeof(int)) < 0){
		closeSocket((int[]){sockfd}, 1, "ERROR on setsockopt");
	}

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		closeSocket((int[]){sockfd}, 1, "ERROR on binding");
	}

	/* Now start listening for the clients, here process will
	   * go in sleep mode and will wait for the incoming connection
	*/

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	if(fork() > 0){
		close(sockfd);
		while(getchar() != 'q'){
		}
		shutdown(sockfd, SHUT_RDWR);
		exit(0);
	}

	while(1) {

		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
			closeSocket((int[]){sockfd, newsockfd}, 1, "ERROR on accept");
		}

		switch(fork()) {
			case -1:
				perror("ERROR on fork");
				break;
			case 0:
				close(sockfd);
				/* If connection is established then start communicating */
				buffer = readAll((int[]){newsockfd, sockfd});

				printf("Here is the message: %s\n", buffer);

				/* Write a response to the client */
				sendAll((int[]){newsockfd, sockfd}, "I got your message");

				closeSocket((int[]){newsockfd}, 0, "");
				break;
			default:
				close(newsockfd);
			}

	}

	closeSocket((int[]){sockfd, newsockfd}, 0, "");

	return 0;
}


