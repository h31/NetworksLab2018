//#include <stdlib.h>
//#include <stdlib.h>

//#include <netdb.h>
//#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#define OK 0
#define ERROR_READING_FROM_SOCKET 1
#define READING_IS_NOT_FINISHED 2
#define REQUEST_LENGTH_ERROR 3

// Command from client
struct command {
	int type;
	char* arg1;
	char* arg2;
};

// Request from client
struct request {
	struct command comm;
	char* token;
};

// Function for creating listen socket
// Returns socket descriptor
int create_listen_socket();

// Close socket for read and write and print message
void close_socket(int sockfd, char* msg);

int get_request(int sockfd, struct request *req);

int read_from(int sockfd, char* buffer, int length);
