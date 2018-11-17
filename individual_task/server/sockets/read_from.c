#include "sockets.h"

int read_from(int sockfd, char* buffer, int length) {
	int read_length = length;
	int n;
	bzero(buffer, length);
	
	while(read_length > 0) {
		n = read(sockfd, buffer, read_length);
		read_length -= n;
		if (n < 0) {
        	//mylog("ERROR reading length of the message from socket");
        	return ERROR_READING_FROM_SOCKET;
    	}
    	if(n == 0) {
			if(read_length > 0) {
				//mylog("Error. Not all bytes have been read");
				return READING_IS_NOT_FINISHED;
			}
		}
	}
	
	return OK;
}

