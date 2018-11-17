#include "sockets.h"

void close_socket(int sockfd, char* msg) {
	printf("Close socket with fd=%d: %s\n", sockfd, msg);
	shutdown(sockfd, 2);
	close(sockfd);
}
