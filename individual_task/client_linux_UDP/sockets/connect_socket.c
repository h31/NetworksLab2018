//
// Created by mrsandman on 09.12.18.
//

#include "socket.h"

#define SERVER "127.0.0.1"
#define PORT 8080

int connect_socket(struct sockaddr_in* serv_addr)
{
    int sockfd;

    if ( (sockfd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }

    memset((char*)serv_addr, 0, sizeof(serv_addr));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);

    if (inet_aton(SERVER , &serv_addr->sin_addr) == 0) {
        perror("inet_aton() failed\n");
        return -1;
    }

    return sockfd;
}