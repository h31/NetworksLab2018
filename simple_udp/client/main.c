#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <string.h>

void closeSocket (int sock);

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;

    char* buffer = (char*)calloc(256, sizeof(char));

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

    n = sendto(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sockfd);
        
        exit(1);
    }
    free(buffer);

    /* Now read server response */
    n = recvfrom(sockfd, buffer, 255, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

        if (n < 0) {
        perror("ERROR reading from socket");
        closeSocket(sockfd);
        
        exit(1);
    }

    closeSocket(sockfd);
    free(buffer);

    return 0;
}

void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}
