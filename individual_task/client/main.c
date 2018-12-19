#include "includes.h"

int main(int argc, char** argv) {
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char* buffer = (char*)calloc(MAX_BUFFER, sizeof(char));
    bzero((char *) &buffer, sizeof(buffer));

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

    /*Now start to work with the server */
    while (true) {
        buffer = readMessage(sockfd);
        switch (buffer[0]) {
            case '0':
                //Write from server; to client it is read
                buffer = readMessage(sockfd);
                printf("%s", buffer);
                break;
            case '1':
                //Read from server; to client it is write
                fgets(buffer, MAX_BUFFER, stdin);
                writeMessage(sockfd, buffer);
                break;
            default:
                free(buffer);
                closeSocket(sockfd);
                return 0;
        }

    }

}