#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket(int socks[], int lenght, int error, char* errorMsg);
char* readAll(int socks[]);
void sendAll(int socks[], char* buffer);

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

	if(setsockopt(sockfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),&(int){ 1 }, sizeof(int)) < 0){
		closeSocket((int[]){sockfd}, 1, 1, "ERROR on setsockopt");
	}

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		int temp[] = {sockfd};
		closeSocket(temp, 1, 1, "ERROR on binding");
	}

	/* Now start listening for the clients, here process will
	   * go in sleep mode and will wait for the incoming connection
	*/

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	if(fork() > 0){
		while(getchar() != 'q'){
		}
		int temp[] = {sockfd};
		closeSocket(temp, 1, 0, "");
	}

	while(1) {

		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
			closeSocket((int[]){sockfd, newsockfd}, 2, 1, "ERROR on accept");
		}

		switch(fork()) {
			case -1:
				perror("ERROR on fork");
				break;
			case 0:
				close(sockfd);
				/* If connection is established then start communicating */
				buffer = readAll((int[]){sockfd, newsockfd});

				printf("Here is the message: %s\n", buffer);

				/* Write a response to the client */
				sendAll((int[]){sockfd, newsockfd}, "I got your message");

				closeSocket((int[]){newsockfd}, 1, 0, "");
				break;
			default:
				close(newsockfd);
			}

	}

	closeSocket((int[]){sockfd, newsockfd}, 2, 0, "");

	return 0;
}

void closeSocket(int socks[], int lenght, int error, char* errorMsg){
	if(strcmp(errorMsg,"") != 0){
		perror(errorMsg);
	}
	for(int i = 0; i < lenght; i++) {
		shutdown(socks[i], SHUT_RDWR);
		close(socks[i]);
	}
	exit(error);
}

char* readAll(int socks[]){
	char *buffer = (char*)malloc(256);
	char strLenght[4];

	int n = read(socks[1], strLenght, 4); // recv on Windows
	if (n < 0) {
		closeSocket(socks, 2, 1, "ERROR reading from socket");
	}
	int lenght = ((strLenght[0] - '0') << 24) + ((strLenght[1] - '0') << 16) + ((strLenght[2] - '0') << 8) + (strLenght[3] - '0');

	int recieved = 0;
	while(recieved < lenght){
		n = read(socks[1], buffer, 256); // recv on Windows
		recieved += n;
		if (n < 0) {
			closeSocket(socks, 2, 1, "ERROR reading from socket");
		}
	}

	return buffer;
}

void sendAll(int socks[], char* buffer){
	int messageLength = strlen(buffer);
	char toSend[5 + 256];
	toSend[0] = ((messageLength >> 24) & 0xff) + '0';
	toSend[1] = ((messageLength >> 16) & 0xff) + '0';
	toSend[2] = ((messageLength >> 8) & 0xff) + '0';
	toSend[3] = ((messageLength >> 0) & 0xff) + '0';

	strcat(toSend, buffer);

	int n = write(socks[1], toSend, strlen(toSend));
	if (n < 0) {
		closeSocket(socks, 2, 1, "ERROR writing to socket");
	}
}
