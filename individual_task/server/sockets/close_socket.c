#include "sockets.h"

void close_socket(int sockfd, char* msg)
{
    mlogf("Close socket with fd=%d: %s", sockfd, msg);
    shutdown(sockfd, 2);
    close(sockfd);
}
