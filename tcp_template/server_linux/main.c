#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

/* function for messaging thread */
void *communicate_func (void *arg);

/*function for listen thread */
void *listen_func (void *arg);

int main(int argc, char *argv[]) {
    pthread_t listen_thread;
    int result;	//result of creating thread
    result = pthread_create(&listen_thread, NULL, listen_func, NULL); //create listen thread
    if(result != 0) {
	perror("Error while creating listen thread");
    }
    
    /* if pressed q then exit program */
    int key = 0;
    while(1) {
	key = getchar();
	if(key == 'q') break;
    }
    
    exit(0);
}

/* waiting for connection */
void *listen_func (void *arg) {
    int sockfd, newsockfd;
    uint16_t portno;
    unsigned int clilen;
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

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    /* Accept connections forever */
    while(1) {
    	/* Accept actual connection from the client */
    	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    	if (newsockfd < 0) {
	    perror("ERROR on accept");
            exit(1);
    	}

	/* Making new thread for messaging with client */
	pthread_t thread;	//thread
	int result;		//result of thread creating
	result = pthread_create(&thread, NULL, communicate_func, &newsockfd); //create new thread
	if(result != 0) {
	    perror("Error while creating thread");
	}
    }

    close(sockfd);
    pthread_exit(0);
}

/* function for thread */
void *communicate_func (void *arg) {
    int newsockfd = * (int *) arg;
    char buffer[256];
    ssize_t n;
    bzero(buffer, 256);
    printf("Here is the message: \n");

    /* If connection is established then start communicating */
   // while(1) {
    	n = recv(newsockfd, buffer, 255, 0); // recv on Windows

	if (n < 0) {
	    perror("ERROR reading from socket");
	    pthread_exit(1);
	}
	
//	if (n == 0) {
//	    break;
//	}
	printf("%s", buffer);
   // }

    /* Write a response to the client */
    n = write(newsockfd, "I got your message", 18); // send on Windows

    if (n < 0) {
        perror("ERROR writing to socket");
	pthread_exit(1);
    }

    close(newsockfd); //close socket after messaging
    pthread_exit(0);
}
