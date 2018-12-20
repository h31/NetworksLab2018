#include "sockets.h"

int create_listen_socket()
{
    int sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;

    // Create new socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    // Initialize socket structure
    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    // Set SO_REUSEPORT and SO_REUSEADDR options
    if (setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &(int) { 1 },
            sizeof(int))
        < 0) {
        close_socket(sockfd, "ERROR on setsockopt");
        exit(1);
    }

    // Bind socket to host address
    if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close_socket(sockfd, "ERROR on binging");
        exit(1);
    }

    // Start listening with backlog = 5
    listen(sockfd, 5);

    return sockfd;
}
