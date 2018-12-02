#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define OPEN 0
#define CLOSED 1

int sockfd;

void closeApp() {
	printf("\nClosing client\r\n");
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[]) {
	int isClose = OPEN;
	signal(SIGINT,closeApp);
    int n;
	int connect;
	int clilen;
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
    serv_addr.sin_port = htons(portno);

    clilen = sizeof(serv_addr);
	
    while(!isClose){
	    printf("Please enter the message: ");
	    bzero(buffer, sizeof(buffer));
	    fgets(buffer, sizeof(buffer)-1, stdin);
	    if (strstr(buffer, "\\q") != NULL) 
		{
			closeApp();
	    }
	    /* Send message to the server */
		n = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, clilen);

	    if (n < 0) 
		{
			perror("ERROR writing sendto");
			closeApp();
	    }
		
	    /* Now read server response */
	    bzero(buffer, 256);
	    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, &clilen);

		if (strstr(buffer, "Server closed. Don't send anything, please") != NULL)
		{
			isClose = CLOSED;
		}

	    if (n <= 0) 
		{
			perror("ERROR reading from recv");
			closeApp();
	    }

	    printf("%s\n", buffer);
    }
	printf("\nClosing client\r\n");
	closeApp();
    return 0;
}