//
// Created by mrsandman on 09.12.18.
//
#include "socket.h"

int read_socket(int sockfd, char* buffer, int length)
{
    int read_length = length;
    ssize_t n;
    bzero(buffer, length);

    while (read_length > 0) {
        n = recv(sockfd, buffer, read_length, NULL);
        read_length -= n;
        if (n < 0) {
            return READING_ERROR;
        }
        if (n == 0) {
            if (read_length > 0) {
                return READING_IS_NOT_FINISHED;
            }
        }
    }

    return WORKING_SOCKET;
}
