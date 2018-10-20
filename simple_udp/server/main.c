#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <pthread.h>

int sockfd;
int serverWorking;

void closeSocket(int sockfd) {
    printf("Close socket\n");
    shutdown(sockfd, 2);
    close(sockfd);
}

void endThread(int sockfd) {
    printf("Thread exit\n");
    closeSocket(sockfd);
    pthread_exit(0);
}

void *communicateWithClients() {
    char buffer[256];
    char answer[] = "I got your message";
    unsigned int clilen;
    struct sockaddr_in cli_addr;
    char* packet_addr;
    ssize_t n;

    printf("Communication with clients starts\n");

    clilen = sizeof(cli_addr);

    while(serverWorking){
        bzero(buffer, 256);
        n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&cli_addr, &clilen);

        if (!serverWorking) {
            endThread(sockfd);
        }

        if (n < 0) {
            perror("ERROR reading from socket");
            continue;
        }

        packet_addr = inet_ntoa(cli_addr.sin_addr);
        if (packet_addr == NULL) {
            perror("ERROR on inet_ntoa");
        }


        printf("Message from: %s\n", packet_addr);
        printf("Here is the message: %s\n", buffer);

        n = sendto(sockfd, answer, strlen(answer), 0,
                    (const struct sockaddr *)&cli_addr, clilen);
        if (n < 0) {
            perror("ERROR writing to socket");
        }
    }
    endThread(sockfd);

    return 0;
}

int main() {
    int status;
    uint16_t portno;
    struct sockaddr_in serv_addr;

    printf("Server starts..\n");
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

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        closeSocket(sockfd);
        exit(1);
    }

    serverWorking = 1;

    pthread_t communicationThread;
    status = pthread_create(&communicationThread, NULL,
                             communicateWithClients, NULL);
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
    printf("\nBYE\n");
    sleep(1);

    return 0;
}

