#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <pthread.h>

int sockfd;//, newsockfd;
int serverListening = 1;

void closeSocket(int sockfd) {
    printf("Close socket\n\n");
    shutdown(sockfd, 2);
    close(sockfd);
}

void endThread(int sockfd) {
    printf("Thread exit\n");
    closeSocket(sockfd);
    pthread_exit(0);
}

void *communicateWithNewClient(void *args) {
    /* If connection is established then start communicating */
    printf("Communication with client starts\n");

    int newsockfd = *(int*) args;
    char buffer[256];
    ssize_t realMsgLen;
    ssize_t curMsgLen;
    char msg[256];
    ssize_t n;

    bzero(buffer, 256);
    bzero(msg, 256);

    n = read(newsockfd, buffer, 255); // recv on Windows

    if (n < 0) {
        perror("ERROR reading from socket");
        endThread(newsockfd);
        return 0;
    }

    realMsgLen = buffer[0];
    curMsgLen = n;
    printf("Message size = %zu, current size = %zu \n", realMsgLen, curMsgLen);
    strcat(msg, buffer + 1);

    while(realMsgLen > curMsgLen) {
        n = read(newsockfd, buffer + curMsgLen, realMsgLen - curMsgLen); // recv on Windows
        if (n < 0) {
            perror("ERROR reading from socket");
            endThread(newsockfd);
            return 0;
        }
        strcat(msg, buffer + curMsgLen);
        curMsgLen += n;
        printf("Message size = %zu, current size = %zu \n", realMsgLen, curMsgLen);
        //strcat(msg, buffer + n);
    }

    printf("Here is the message: %s\n", msg);

    /* Write a response to the client */
    n = write(newsockfd, "I got your message", 18); // send on Windows

    if (n < 0) {
        perror("ERROR writing to socket");
        endThread(newsockfd);
        return 0;
    }

    endThread(newsockfd);

    return 0;
}

void *listenForConnection() {
    printf("Listening\n");

    int newsockfd, status;
    unsigned int clilen;
    struct sockaddr_in cli_addr;

    if (listen(sockfd, 5) < 0) {
        perror( "listen error!\n");
        return 0;
    }

    clilen = sizeof(cli_addr);

    pthread_t communicationThread;

    while (serverListening) {

        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (!serverListening) {
            break;
        }

        if (newsockfd < 0) {
            perror("ERROR on accept");
            break;//endThread(sockfd);
        }

        printf("Accept connection\n");

        status = pthread_create(&communicationThread, NULL, communicateWithNewClient,&newsockfd);
        if (status != 0) {
            printf("Can't create thread, status = %d\n", status);
            //exit(1);
        }
    }

    endThread(sockfd);

    return 0;
}

int main(int argc, char *argv[]) {
    int status;
    uint16_t portno;
    struct sockaddr_in serv_addr;

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
        closeSocket(sockfd);
        exit(1);
    }

    pthread_t listeningThread;
    status = pthread_create(&listeningThread, NULL, listenForConnection, NULL);
    if (status != 0) {
        printf("Can't create thread, status = %d\n", status);
        closeSocket(sockfd);
        exit(1);
    }

    int checkKey = 0;

    while (1) {
        printf(">>>>>>>>WRITE q TO EXIT<<<<<<<<\n\n");
        if ((checkKey = getchar()) == 'q') {
            serverListening = 0;
            break;
        }
    }

    closeSocket(sockfd);
    printf("BYE.\n");

    return 0;
}

