#include "sockets.h"

void close_socket(int sockfd, char *msg) {
  perror(msg);
  shutdown(sockfd, 2);
  close(sockfd);
}
