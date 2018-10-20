#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>

void errClose(int sockfd, char* error) {
    perror(error);
    close(sockfd);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    char buffer[256];
    unsigned int len = sizeof(serv_addr);

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
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    n = sendto(sockfd, buffer, strlen(buffer), 0,
                    (struct sockaddr *)&serv_addr, len);
    if (n < 0) {
        errClose(sockfd, "ERROR writing to socket");
    }

    /* Now read server response */
    bzero(buffer, 256);

    n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&serv_addr, &len);
    if (n < 0) {
        errClose(sockfd, "ERROR reading from socket");
    }

    printf("%s\n", buffer);

    close(sockfd);

    return 0;
}

