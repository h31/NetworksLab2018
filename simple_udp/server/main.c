#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <unistd.h>

/* function for listen thread */
void *listen_func (void *arg);

/* variable for enable/disable logging */
int log_enabled = 0;

/* function for logging */
void logv(char* msg, int value);

void log(char* msg);

struct socket_data {
    int sockfd;
    unsigned int clilen;
    struct sockaddr_in cli_addr;
};

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    struct socket_data data;

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
    
    if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), & (int) {1}, sizeof(int)) < 0) {
    	perror("ERROR on setsockopt");
    	shutdown(sockfd, 2);
    	close(sockfd);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        shutdown(sockfd, 2);
    	close(sockfd);
        exit(1);
    }

    clilen = sizeof(cli_addr);

    /* data which will be sent in listen thread */
    data.sockfd = sockfd;
    data.clilen = clilen;
    data.cli_addr = cli_addr;

    pthread_t listen_thread; //thread for listening
    int result;	//result of creating thread
    result = pthread_create(&listen_thread, NULL, listen_func, &data); //create listen thread
    if(result != 0) {
	perror("Error while creating listen thread");
	shutdown(sockfd, 2);
	close(sockfd);
	exit(1);
    }
    
    /* if pressed q then exit program */
    int key = 0;
    while(1) {
	key = getchar();
	if(key == 'q') break;
	// enable/disable logging
	if(key == 'l') {
	    log_enabled = !log_enabled;
	    log("log enabled");
	}
    }
    shutdown(sockfd, 2);
    close(sockfd);
    exit(0);
}

/* waiting for connection */
void *listen_func (void *arg) {
    struct socket_data data = * (struct socket_data *) arg;
    char buffer[256];
    int n;
    while(1) {
	n = recvfrom(data.sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &data.cli_addr, &data.clilen);
	if(n == -1) {
	    perror("ERROR on receive data");
	    shutdown(data.sockfd, 2);
	    close(data.sockfd);
	    exit(1);
	} else {
	    printf("Here is the message:%s\n", buffer);
	    n = sendto(data.sockfd, "I got your message", 18, 0, (struct sockaddr *) &data.cli_addr, data.clilen);
	    if(n == -1) {
		perror("ERROR on send data");
		shutdown(data.sockfd, 2);
		close(data.sockfd);
		exit(1);
	    }
	    log("Message sent\n");
	}
    }

    pthread_exit(0);
}

void logv(char* msg, int value) {
    if(log_enabled) {
        printf("log: %s %d\n", msg, value);
    }
}

void log(char* msg) {
    if(log_enabled) {
	printf("log: %s\n", msg);
    }
}
