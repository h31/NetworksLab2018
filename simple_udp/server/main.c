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

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

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


    clilen = sizeof(cli_addr);
    

    while(1) {

	    bzero(buffer, 256);

	    if ((n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &cli_addr, &clilen)) < 0) {
		perror("ERROR on accept");
		exit(1);
	    }
   		
            if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*) &cli_addr, clilen) < 0) {
		perror("ERROR writing to socket");
		exit(1);
	    }
	    
	 
            printf("%s\n" , buffer);
    }

    return 0;
}
