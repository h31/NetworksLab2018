#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../logging/logging.h"

#define SOCKETS_OK 0
#define ERROR_READING_FROM_SOCKET 101
#define READING_IS_NOT_FINISHED 102
#define REQUEST_LENGTH_ERROR 103

#define GET 1
#define REG 2
#define LOGIN 3
#define LIST 4
#define SEND 5
#define QUIT 6
#define DEL 7

// Command from client
struct command {
	char* type;
	char* arg1;
	char* arg2;
};

// Request from client
struct request {
	struct command comm;
	char* token;
};

// Response to client
struct response {
	char* type;
	char* payload;
};

// Function for creating listen socket
// Returns socket descriptor
int create_listen_socket();

// Close socket for read and write and print message
void close_socket(int sockfd, char* msg);

int get_request(int sockfd, struct request* req);

int read_from(int sockfd, char* buffer, int length);

int send_response(int sockfd, struct response* resp);
