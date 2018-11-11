#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket (int socket);
void writeMessage (int sock, char* buffer);
char* readMessage(int sock);

int main(int argc, char *argv[]) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char* buffer = (char*)calloc(256, sizeof(char));


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
        fprintf(stderr, "ERROR no such host\n");
        closeSocket(sockfd);
        
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        closeSocket(sockfd);
        
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server*/
    printf("Please enter the message: ");
    fgets(buffer, 255, stdin);
    writeMessage(sockfd, buffer);
    free(buffer);

    /* Now read server response */
    buffer = readMessage(sockfd);
    free(buffer);

    closeSocket(sockfd);

    return 0;
}

void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}

void writeMessage (int sock, char* buffer) {

    uint32_t messlen;
    char* bufForLen = (char*)calloc(4, sizeof(char));

    /* Send length of message to the server */
    messlen = strlen(buffer);
    sprintf(bufForLen, "%04d", messlen);
    int n = write(sock, bufForLen, strlen(bufForLen));

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(sock);
        
        exit(1);
    }
    free(bufForLen);

    /* Send message to the server */
    n = write(sock, buffer, messlen);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sock);
        
        exit(1);
    }
}

char* readMessage(int sock) {

    char* buffer = (char*)calloc(256, sizeof(char));
    char* bufForLen = (char*)calloc(4, sizeof(char));
    uint32_t messlen;

    /*Read lenght of message from the server*/
    ssize_t n = read(sock, bufForLen, 4); 

    if (n < 0) {
        perror("ERROR lenght of message");
        closeSocket(sock);
        
        exit(1);
    }
    
    messlen = atol(bufForLen);
    free(bufForLen);

    /*Read message from the server*/
    for(unsigned int i = 0; i < messlen; i += n) {

        n = read(sock, buffer + i, 255); 

        if (n < 0) {
            perror("ERROR of message");
            closeSocket(sock);
            
            exit(1);
        }
    }

    printf("%s\n", buffer);

    return buffer;
}