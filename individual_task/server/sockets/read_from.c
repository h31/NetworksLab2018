#include "sockets.h"

int read_from(int sockfd, char* buffer, int length)
{
    int read_length = length;
    int buffer_index = 0;
    ssize_t n;
    bzero(buffer, length);

    mlogf("Start reading %d bytes from %d socket", length, sockfd);

    while (read_length > 0) {
        n = read(sockfd, &buffer[buffer_index], read_length);
        buffer_index += n;
        read_length -= n;
        mlogf("Read %d bytes, left %d bytes", n, read_length);
        if (n < 0) {
            mlogf("Error while reading from socket. %d/%d bytes was read, n = %s", length - read_length, length, n);
            return ERROR_READING_FROM_SOCKET;
        }
        if (n == 0) {
            if (read_length > 0) {
                mlogf("Reading is not finished. %d/%d bytes was read",
                    length - read_length, length);
                return READING_IS_NOT_FINISHED;
            }
        }
    }

    mlogf("OK. Sockfd = %d, read_length = %d", sockfd, read_length);

    return SOCKETS_OK;
}
