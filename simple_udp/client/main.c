#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string.h>

void closeSocket (int sock);

int main(int argc, char *argv[]) {
    int sockfd;
    ssize_t n;
    uint16_t portno;
    unsigned int servlen;
    struct sockaddr_in serv_addr;

    char* buffer = (char*)calloc(256, sizeof(char));
    bzero(buffer, 256);

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);

    /* Now ask for a message from the user, this message
       * will be read by server*/
    printf("Please enter the message: ");
    fgets(buffer, 255, stdin);

    servlen = sizeof(serv_addr);

    n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &serv_addr, servlen);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sockfd);
        
        exit(1);
    }

    bzero(buffer, 256);
 
    /* Now read server response */
    n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &serv_addr, &servlen);

        if (n < 0) {
        perror("ERROR reading from socket");
        closeSocket(sockfd);
        
        exit(1);
    }

    printf("%s\n", buffer);

    closeSocket(sockfd);
    free(buffer);

    return 0;
}

void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}
