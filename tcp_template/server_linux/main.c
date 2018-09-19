#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket(int socks[], int lenght, int error, char* errorMsg);

int main(int argc, char *argv[]) {
	int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
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

    if(setsockopt(sockfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),&(int){ 1 }, sizeof(int)) < 0){
		int temp[] = {sockfd};
	    closeSocket(temp, 1, 1, "ERROR on setsockopt");
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
            int temp[] = {sockfd, newsockfd};
	    	closeSocket(temp, 2, 1, "ERROR on accept");
        }

		switch(fork()) {
		    case -1:
				perror("ERROR on fork");
				break;
			case 0:
				close(sockfd);
				/* If connection is established then start communicating */
				bzero(buffer, 256);
				n = read(newsockfd, buffer, 255); // recv on Windows

				if (n < 0) {
					int temp[] = {sockfd, newsockfd};
					closeSocket(temp, 2, 1, "ERROR reading from socket");
				}

				printf("Here is the message: %s\n", buffer);

				/* Write a response to the client */
				n = write(newsockfd, "I got your message", 18); // send on Windows

				if (n < 0) {
					int temp[] = {sockfd, newsockfd};
					closeSocket(temp, 2, 1, "ERROR writing to socket");
				 }
				int temp[] = {newsockfd};
				closeSocket(temp, 1, 0, "");
				break;
			default:
				close(newsockfd);
			}

	}

    int temp[] = {sockfd, newsockfd};
	closeSocket(temp, 2, 0, "");

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
