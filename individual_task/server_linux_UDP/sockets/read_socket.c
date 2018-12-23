//
// Created by mrsandman on 09.12.18.
//
#include "socket.h"

int read_socket(int sockfd, char* buffer, int length, struct sockaddr_in * cli_addr)
{
    int read_length = length;
    int slen = sizeof(*cli_addr);
    ssize_t n;
    bzero(buffer, read_length);

    while (read_length > 0) {
        n = recvfrom(sockfd, buffer, read_length, MSG_WAITALL, cli_addr, &slen);
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
