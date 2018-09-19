#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket(int sockfd, int error, char* errorMsg);

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        closeSocket(sockfd, 0, "ERROR, no such host\n");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        closeSocket(sockfd, 1, "ERROR connecting");
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    n = write(sockfd, buffer, strlen(buffer));

    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR writing to socket");
    }

    /* Now read server response */
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);

    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR reading from socket");
    }

    printf("%s\n", buffer);

    closeSocket(sockfd, 0, "");

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
