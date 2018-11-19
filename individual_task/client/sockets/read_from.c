#include "sockets.h"

int read_from(int sockfd, char *buffer, int length) {
  int read_length = length;
  ssize_t n;
  bzero(buffer, length);

  while (read_length > 0) {
    n = read(sockfd, buffer, read_length);
    read_length -= n;
    if (n < 0) {
      return ERROR_READING_FROM_SOCKET;
    }
    if (n == 0) {
      if (read_length > 0) {
        printf("Reading is not finisher. %d/%d bytes was read",
               length - read_length, length);
        return READING_IS_NOT_FINISHED;
      }
    }
  }

  return OK;
}
