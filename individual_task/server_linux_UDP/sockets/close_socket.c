//
// Created by mrsandman on 09.12.18.
//
#include "socket.h"

void close_socket(int sockfd, char* msg)
{
    perror(msg);
    shutdown(sockfd, 2);
    close(sockfd);
}
