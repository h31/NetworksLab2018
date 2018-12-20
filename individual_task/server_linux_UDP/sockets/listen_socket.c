//
// Created by mrsandman on 09.12.18.
//
#include "socket.h"

#define PORT 8080

int listen_socket()
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // Create new socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    memset((char*)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);

    // Set SO_BROADCAST options
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &(int) { 1 },sizeof(int))< 0) {
        close_socket(sockfd, "ERROR on setsockopt");
        exit(1);
    }

    // Bind socket to host address
    if (bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
        close_socket(sockfd, "ERROR on binging");
        exit(1);
    }

    return sockfd;
}
