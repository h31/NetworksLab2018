#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket(int sockfd, int error, char* errorMsg);
int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno = 5001;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    unsigned int clilen = sizeof(cli_addr);
    ssize_t n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if(setsockopt(sockfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),&(int){ 1 }, sizeof(int)) < 0){
        closeSocket(sockfd, 1, "ERROR on setsockopt");
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        closeSocket(sockfd, 1, "ERROR on binding");
    }

    if(fork() > 0){
        while(getchar() != 'q'){
        }
        closeSocket(sockfd, 0, "");
    }

    while (1) {
        bzero(buffer, 256);
        n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (n < 0) {
            closeSocket(sockfd, 1, "ERROR reading from socket");
        }
        printf("Here is the message: %s\n", buffer);

        n = sendto(sockfd, "I got your message", 18, 0, (struct sockaddr *) &cli_addr, clilen);
        if (n < 0) {
            closeSocket(sockfd, 1, "ERROR writing to socket");
        }
    }

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
