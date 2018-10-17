#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h> //for threading
#include <string.h>

int sockfd;
int clients = 0;

//the thread function
void *connection_handler(void *);

void close_sock(int sockfd) {
    printf("Server is closing\r\n");
    while (clients !=0){
         printf("Closing clients\r\n");
         sleep(1);
    }
    printf("Server is closing\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void thread_close(int newsockfd){
    printf("Thread closing\r\n");
    shutdown(newsockfd, SHUT_RDWR);
    close(newsockfd);
    clients--;
    pthread_exit(NULL);
}

void thread_request(int socket){
    requestDataStruct rds;
    pthread_t theThread;
    int status;

    rds.socket = socket;

    status = pthread_create(&theThread, NULL, connection_handler, (void*) &rds);
    if (status != 0) {
    printf("Can't create a thread with status: %d\n", status);
    exit(2);
    }
}
    

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, *new_sock;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        close_sock(sockfd);
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

    /* Accept actual connection from the client */
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

    if (newsockfd < 0) {
        perror("ERROR on accept");
        close_sock(sockfd);
        exit(1);
    }
    
    thread_request(newsockfd);

    /* If connection is established then start communicating 
    bzero(buffer, 256);
    n = read(newsockfd, buffer, 255); // recv on Windows

    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }

    printf("Here is the message: %s\n", buffer);

    /* Write a response to the client 
    n = write(newsockfd, "I got your message", 18); // send on Windows

    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    } */

    close_sock(sockfd);
    return 0;
}

void *connection_handler(void *sockfd)
      {
    //Get the socket descriptor
    int sock = *(int*)sockfd;
    int size;
    char buffer[256];
    
    clients++;

    //Receive a message from client
    while( (size = read(sock , buffer , 256 )) > 0 )
       {
        //Send the message back to client
        write(sock , buffer , strlen(buffer));
        printf("%s\n",buffer);
       }

    if(size == 0)
       {
        puts("Client disconnected");
        fflush(stdout);
       }
    else if(size == -1)
       {
        perror("Fail read");
       }

    thread_close(sock);
    return;
}



