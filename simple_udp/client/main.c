#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include<arpa/inet.h>

#include <string.h>

void closeSocket(int sockfd, int error, char* errorMsg);
int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    unsigned int servlen = sizeof(serv_addr);

    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
     
    if (inet_aton(argv[1], &serv_addr.sin_addr) == 0){
        closeSocket(sockfd, 1, "ERROR connecting");
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &serv_addr, servlen);

    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR writing to socket");
    }

    /* Now read server response */
    bzero(buffer, 256);
    n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr *) &serv_addr, &servlen);

    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR reading from socket");
    }

    printf("%s\n", buffer);

    close(sockfd);
    return 0;
}

void closeSocket(int sockfd, int error, char* errorMsg){
    if(strcmp(errorMsg,"") != 0){
        perror(errorMsg);
    }
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(error);
}