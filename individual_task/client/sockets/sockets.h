#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#define OK 0
#define INVALID_TYPE 1

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

// Function for connecting to server
int connect_to(int argc, char** argv);

// Close socket for read and write and print message
void close_socket(int sockfd, char* msg);

// Wrap request into byte array and send to client
int send_request(int sockfd, struct request *req);

// Create request from string str
int parse_request(char* str, struct request *req);
