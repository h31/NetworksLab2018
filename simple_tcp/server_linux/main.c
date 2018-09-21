#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

#define OPEN 0
#define CLOSED 1

void *connection_f ();
void *connect_handler(void* args);

int sockfd;

int max_clients = 2;
int all_clients = 0;
int isClose = OPEN;

void clear_all(){
	isClose = CLOSED;
	printf("closing server\r\n");
	while (all_clients != 0){
		printf("Waiting for closing clients... \r\n");
		sleep(1);
	}
	printf("Closing server...\n");
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
}

void deleteConnection(int newsockfd){
	printf("Closing socket = %d \r\n", newsockfd);
	shutdown(newsockfd, SHUT_RDWR);
	close(newsockfd);
	all_clients--;
	pthread_exit(0);
}
int sockfd;

int main(int argc, char *argv[]) {
	//CTRL+C
	signal(SIGINT, clear_all);
	
    uint16_t portno;
    struct sockaddr_in serv_addr;


    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd <= 0) {
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

	/* Making new thread for messaging with client */
	pthread_t thread;	//thread
	int result;		//result of thread creating
	result = pthread_create(&thread, NULL, connection_f, NULL); //create new thread
	if(result != 0) {
	    perror("Error while creating thread");
	}

	char cmd;
	
	while(!isClose){
		cmd = getchar();
		if (cmd == 'q'){
			break;
		}
	}
    clear_all();
    return 0;
}

void *connection_f (){
    /* Accept actual connection from the client */
    struct sockaddr_in cli_addr;
	unsigned int clilen;
	int newsockfd;
    int n;
	
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
	
	pthread_t threadCreation;
	
	while (!isClose) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd <= 0) {
        	perror("ERROR on accept");
        	continue;
    	}
		n = pthread_create(&threadCreation, NULL, connect_handler, (void*) newsockfd);
		if (n != 0) {
			printf("Error on creating client thread");
		} else {
			all_clients++;
		}
	}
	
	pthread_exit(0);
}

void *connect_handler (void *args){
	int newsockfd = (int*) args;
	char buffer[256];
	ssize_t n;
	
	/* If connection is established then start communicating */
    while(!isClose){
		bzero(buffer, sizeof(buffer));
	    n = read(newsockfd, buffer, sizeof(buffer)); // recv on Windows
	    if (n <= 0) {
			perror("ERROR reading from socket");
			deleteConnection(newsockfd);
	    }
	    printf("Here is the message: %s\n", buffer);

	    /* Write a response to the client */
	    n = write(newsockfd, "I got your message", 18); // send on Windows

	    if (n <= 0) {
			perror("ERROR writing to socket");
			deleteConnection(newsockfd);
	    }
	    bzero(buffer, 256);
	    
    }
	deleteConnection(newsockfd);


}
