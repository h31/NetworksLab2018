#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <unistd.h>

/* function for messaging thread */
void *communicate_func (void *arg);

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

    /* Now start listening for the clients, here process will
     * go in sleep mode and will wait for the incoming connection */

    listen(sockfd, 5);
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
    sleep(1);
    exit(0);
}

/* waiting for connection */
void *listen_func (void *arg) {
    struct socket_data data = * (struct socket_data *) arg;
    int newsockfd;
    
    /* Accept connections while server is on */
    while(1) {
    	/* Accept actual connection from the client */
    	newsockfd = accept(data.sockfd, (struct sockaddr *) &data.cli_addr, &data.clilen);
    	
    	if (newsockfd < 0) {
		perror("ERROR on accept");
		pthread_exit(1);
    	}

	logv("new connection accepted with fd=", newsockfd);

	/* Making new thread for messaging with client */
	pthread_t thread; //thread for messaging
	int result;	//result of thread creating
	result = pthread_create(&thread, NULL, communicate_func, &newsockfd); //create new thread
	if(result != 0) {
		perror("Error while creating thread");
		shutdown(newsockfd, 2);
		close(newsockfd);
	}
		
    }
    pthread_exit(0);
}

/* function for thread */
void *communicate_func (void *arg) {
    int newsockfd = * (int *) arg;
    char buffer[256];
    char output[300]; //put message in buffer 
    ssize_t n;
    int message_length;
    int received_length = 0;
    int read_length = 0;

    bzero(output, 300);

    strcat(output, "Here is the message: ");

    /* If connection is established then start communicating 
       First we need to read length of message */
    n = read(newsockfd, (char*)&message_length, sizeof(int)); // recv on Windows
    
    logv("length = ", message_length);

    if (n < 0) {
        perror("ERROR reading length of the message from socket");
        shutdown(newsockfd, 2);
        close(newsockfd);
        pthread_exit(1);
    }

    while (received_length < message_length) {
    	bzero(buffer, 256);
	read_length = message_length - received_length;
	if(read_length > 255) read_length = 255;
	n = read(newsockfd, buffer, read_length);
        if (n < 0) {
		perror("ERROR reading from socket");
	    	shutdown(newsockfd, 2);
	    	close(newsockfd);
	    	pthread_exit(1);
	}
	//n == 0 means that we read all bytes or client has closed connection
	if(n == 0) {
		if(received_length < message_length) {
			printf("Error. Not all bytes have been read\n");
		} else {
			printf("All bytes have been read\n");
		}
		break;
	}
	received_length += n;
	strcat(output, buffer);

	logv("received bytes: ", n);
    }
    printf("%s\n", output);

    /* Write a response to the client */
    n = write(newsockfd, "I got your message", 18); // send on Windows

    if (n < 0) {
        perror("ERROR writing to socket");
        shutdown(newsockfd, 2);
        close(newsockfd);
	pthread_exit(1);
    }
    
    log("closing connection");

    shutdown(newsockfd, 2);
    close(newsockfd); //close socket after messaging
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
