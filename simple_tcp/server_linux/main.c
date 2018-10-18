#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <pthread.h>

int sockfd;//, newsockfd;
int serverWorking = 1;

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
    int msgSize;
    char buffer[256];
    ssize_t realMsgLen;
    ssize_t curMsgLen;
    ssize_t n;

    bzero(buffer, 256);

    n = read(newsockfd, buffer, 255); // recv on Windows

    if (n <= 0) {
        perror("ERROR reading from socket");
        endThread(newsockfd);
        return 0;
    }

    realMsgLen = buffer[0] + 1;
    curMsgLen = n;
    printf("Message size = %zu, current size = %zu \n", realMsgLen, curMsgLen);

    while(realMsgLen > curMsgLen) {
        n = read(newsockfd, buffer + curMsgLen, realMsgLen - curMsgLen); // recv on Windows
        if (n <= 0) {
            perror("ERROR reading from socket");
            endThread(newsockfd);
            return 0;
        }
        curMsgLen += n;
        printf("Message size = %zu, current size = %zu \n", realMsgLen, curMsgLen);
    }

    printf("Here is the message: %s\n", buffer + 1);

    bzero(buffer, 256);
    strcat(buffer + 1, "I got your message");
    msgSize = strlen(buffer + 1);
    buffer[0] = msgSize;

    /* Write a response to the client */
    n = write(newsockfd, buffer, strlen(buffer)); // send on Windows

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

    while (serverWorking) {

        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (!serverWorking) {
            break;
        }

        if (newsockfd < 0) {
            perror("ERROR on accept");
            break;
        }

        printf("Accept connection\n");

        status = pthread_create(&communicationThread, NULL, communicateWithNewClient,&newsockfd);
        if (status != 0) {
            printf("Can't create thread, status = %d\n", status);
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

    while (1) {
        printf(">>>>>>>>WRITE q TO EXIT<<<<<<<<\n\n");
        if (getchar() == 'q') {
            serverWorking = 0;
            break;
        }
    }

    closeSocket(sockfd);
    printf("BYE.\n");
    sleep(1);
    return 0;
}

