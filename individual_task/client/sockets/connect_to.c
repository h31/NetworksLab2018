#include "sockets.h"

int connect_to(int argc, char** argv)
{
    int sockfd;
    struct hostent* server;
    uint16_t portno;
    struct sockaddr_in serv_addr;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        return -1;
    }
    portno = (uint16_t)atoi(argv[2]);

    // Create a socket point
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        return -1;
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        return -1;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char*)&serv_addr.sin_addr.s_addr,
        (size_t)server->h_length);
    serv_addr.sin_port = htons(portno);

    // Now connect to the server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close_socket(sockfd, "ERROR connecting");
        return -1;
    }

    return sockfd;
}
