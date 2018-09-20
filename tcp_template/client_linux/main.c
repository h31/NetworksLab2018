#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket(int sockfd, int error, char* errorMsg);
void sendAll(int sockfd, char buffer[]);
char* readAll(int sockfd);

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char* buffer = (char*)malloc(256);

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
    fgets(buffer, 255, stdin);

    /* Send message to the server */
    sendAll(sockfd, buffer);

    /* Now read server response */
    buffer = readAll(sockfd);

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

void sendAll(int sockfd, char buffer[]){
    int messageLength = strlen(buffer);
    char toSend[5 + 256];
    toSend[0] = ((messageLength >> 24) & 0xff) + '0';
    toSend[1] = ((messageLength >> 16) & 0xff) + '0';
    toSend[2] = ((messageLength >> 8) & 0xff) + '0';
    toSend[3] = ((messageLength >> 0) & 0xff) + '0';

    strcat(toSend, buffer);

    int n = write(sockfd, toSend, strlen(toSend));
    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR writing to socket");
    }
}

char* readAll(int sockfd){
    char *buffer = (char*)malloc(256);
    char strLenght[4];

    int n = read(sockfd, strLenght, 4); // recv on Windows
    if (n < 0) {
        closeSocket(sockfd, 1, "ERROR reading from socket");
    }
    int lenght = ((strLenght[0] - '0') << 24) + ((strLenght[1] - '0') << 16) + ((strLenght[2] - '0') << 8) + (strLenght[3] - '0');

    int recieved = 0;
    while(recieved < lenght){
        n = read(sockfd, buffer, 256); // recv on Windows
        recieved += n;
        if (n < 0) {
            closeSocket(sockfd, 1, "ERROR reading from socket");
        }
    }

    return buffer;
}