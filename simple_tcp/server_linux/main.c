#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket (int socket);
void writeMessage(int newsock, char* buffer);
char* readMessage (int sock, int newsock);

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char* buffer;
    
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

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, & (int) {1}, sizeof(int)) < 0) {
    	perror("ERROR on setsockopt");
    	closeSocket(sockfd);
    	
		exit(1);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        closeSocket(sockfd);

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
		closeSocket(sockfd);		
    }

	while(1) {

		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

		if (newsockfd < 0) {
		    perror("ERROR on accept");
			closeSocket(sockfd);
			
		    exit(1);
		}

		switch(fork()) {

			case -1:
				perror("ERROR on fork");
				break;

			case 0:
				close(sockfd);

				/*Get the message from client*/
				buffer = readMessage(sockfd, newsockfd);
				printf("Here is the message: %s\n", buffer);
				free(buffer);

				/* Write a response to the client */
				writeMessage(newsockfd, "I GOT YOUR MESSAGE");
				closeSocket(newsockfd);
				
				exit(0);

			default:
				close(newsockfd);
		}  
	}
	
	closeSocket(newsockfd);
	closeSocket(sockfd);

    return 0;
}

void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}

char* readMessage (int sock, int newsock) {

	char* buffer = (char*)calloc(256, sizeof(char));
	char* bufForLen = (char*)calloc(4, sizeof(char));
	uint32_t messlen;

	/* Read length of message from the client */
	ssize_t n = read(newsock, bufForLen, 4); 

	if (n < 0) {
		perror("ERROR lenght of message");
		closeSocket(sock);
		closeSocket(newsock);
		
		exit(1);
	}
	
	messlen = atol(bufForLen);
	free(bufForLen);

	/* Read message from the client */
	for(unsigned int i = 0; i < messlen; i += n) {

		n = read(newsock, buffer + i, 255); 

		if (n < 0) {
			perror("ERROR of message");
			closeSocket(sock);
			closeSocket(newsock);
			
			exit(1);
		}
	}
	
	return buffer;

}

void writeMessage(int newsock, char* buffer) {

    uint32_t messlen;
    char* bufForLen = (char*)calloc(4, sizeof(char));

    /* Send length of message to the client */
    messlen = strlen(buffer);
    sprintf(bufForLen, "%04d", messlen);
    int n = write(newsock, bufForLen, strlen(bufForLen));

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(newsock);
        
        exit(1);
    }
    free(bufForLen);

    /* Send message to the client */
    n = write(newsock, buffer, messlen);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(newsock);
        
        exit(1);
    }
}