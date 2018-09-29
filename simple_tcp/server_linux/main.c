#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    long messlen;

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

	if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), & (int) {1}, sizeof(int)) < 0) {
    	perror("ERROR on setsockopt");
    	shutdown(sockfd, SHUT_RDWR);
    	close(sockfd);
		exit(1);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");

		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

	if(fork() > 0){
		while(getchar() != 'q'){
		}
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
    }

	while(1) {

		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
		    perror("ERROR on accept");

			shutdown(sockfd, SHUT_RDWR);
			close(sockfd);
		    exit(1);
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
				perror("ERROR lenght of message");

				shutdown(sockfd, SHUT_RDWR);
				close(sockfd);

				shutdown(newsockfd, SHUT_RDWR);
				close(newsockfd);
				exit(1);
			}
			
			messlen = atol(buffer);

			bzero(buffer, 256);

			for(int i = 0; i < messlen; i += n){
				n = read(newsockfd, buffer + i, 255); 
				if (n < 0) {
					perror("ERROR of message");

					shutdown(sockfd, SHUT_RDWR);
					close(sockfd);

					shutdown(newsockfd, SHUT_RDWR);
					close(newsockfd);
					exit(1);
				}
			}
			
			printf("Here is the message: %s\n", buffer);

			/* Write a response to the client */
			n = write(newsockfd, "I got your message", 18); // send on Windows

			if (n < 0) {
				perror("ERROR writing to socket");

				shutdown(sockfd, SHUT_RDWR);
				close(sockfd);

				shutdown(newsockfd, SHUT_RDWR);
				close(newsockfd);
				exit(1);
			}
			close(newsockfd);
			
			exit(0);
			default:
			close(newsockfd);
		}  
	}
	
	shutdown(newsockfd, SHUT_RDWR);
	close(newsockfd);

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);

    return 0;
}
