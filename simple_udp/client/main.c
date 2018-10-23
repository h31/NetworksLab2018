#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char *argv[]) {
    int sockfd, n, servlen;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    while(1) {
	    printf("Please enter the message: ");
	    bzero(buffer, 256);
	    fgets(buffer, 255, stdin);
	    servlen = sizeof(serv_addr);

	    /* Send message to the server */
	    if (sendto(sockfd, buffer, strlen(buffer) , 0 , (struct sockaddr *) &serv_addr, servlen) < 0) {
		printf("ERROR writing to socket");
		exit(1);
	    }

	    /* Now read server response */
            if (recvfrom(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*) &serv_addr, (socklen_t *) &servlen) < 0) {
 		printf("ERROR recieving");
		exit(1);
            } 

    }
    printf("%s\n", buffer); 
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd); 
    return 0;
}
