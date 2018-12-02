#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define OPEN 0
#define CLOSED 1

int sockfd, n;
int isClose = OPEN;
uint16_t portno;
struct sockaddr_in serv_addr;
struct sockaddr_in cli_addr;
unsigned int clilen;
int sendD;
int rec;
char buffer[256];

void clear_all() {
	isClose = CLOSED;
	char gotMsg[46] = "Server closed. Don't send anything, please";
	int sendi = sendto(sockfd, gotMsg, rec, 0, (struct sockaddr*) &cli_addr, clilen);
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}

int main(int argc, char *argv[]) {
    //CTRL+C
    signal(SIGINT, clear_all);

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
	
    n = bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	
    if (n < 0) {
	perror("error on bind");
	exit(1);
    }
	
    clilen = sizeof(cli_addr);
	
    while(!isClose) {
	    while ((rec = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &cli_addr, &clilen)) > 0) {
		char gotMsg[21] = "I've got your message";
		sendD = sendto(sockfd, gotMsg, rec, 0, (struct sockaddr*) &cli_addr, clilen);
		if (sendD < 0) {
			perror("error on send");
			exit(1);
		}
		printf("Here is the message: %s\n", buffer);
		bzero(buffer, sizeof(buffer));
	    }
    }
    printf("\n Closing server...\r\n");
    clear_all();
    return 0;
}
