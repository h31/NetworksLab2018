#include "sockets.h"

int read_from(int sockfd, char* buffer, int length)
{
    int read_length = length;
    int buffer_index = 0;
    ssize_t n;
    bzero(buffer, length);

    while (read_length > 0) {
        n = read(sockfd, &buffer[buffer_index], read_length);
        buffer_index += n;
        read_length -= n;
        if (n < 0) {
            return ERROR_READING_FROM_SOCKET;
        }
        if (n == 0) {
            if (read_length > 0) {
                printf("Reading is not finished. %d/%d bytes was read\n",
                    length - read_length, length);
                return READING_IS_NOT_FINISHED;
            }
        }
    }

    return OK;
}
