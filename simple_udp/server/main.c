#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h> //for threading
#include <string.h>
#include <arpa/inet.h>

int sockfd;
unsigned int clilen;
struct sockaddr_in cli_addr;

//the thread function
void *connection_handler(void *);

void thread_request(){
    
    pthread_t theThread;
    int status;

    status = pthread_create(&theThread, NULL, connection_handler, NULL);
    if (status != 0) {
    printf("Can't create a thread with status: %d\n", status);
    exit(1);
    }
}

void *thread_control() {
    char n;
    
    do {
         n = getchar();
    } while (n != 'q');
     
    printf("Exiting...\n"); 
    shutdown(sockfd, 2);
    close(sockfd);

    return 0;
}    

int main(int argc, char *argv[]) {
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr;
    ssize_t n;
    int status;
    pthread_t controling;


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

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    status = pthread_create(&controling, NULL, thread_control, NULL);
    if (status != 0) {
    printf("Cant create a control thread. Status: %d\n", status);
    exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    clilen = sizeof(cli_addr);

    while(1) {
    bzero(buffer, 256);
    /* Accept msg from the client */
    n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr *) &cli_addr, &clilen);
    printf("Here is the message: %s\n", buffer);

    thread_request(newsockfd);
    }
}

void *connection_handler(void *sockfd)
    {

    int n;
    struct sockaddr_in cli_addr_local = cli_addr;
    
    n = sendto(sockfd, "I got your message", 18, 0, (struct sockaddr*) &cli_addr_local, clilen); // sendto

    if (n < 0) {
	 perror("ERROR writing to socket");
	 exit(1);
    }

    return 0;
}
