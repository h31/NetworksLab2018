//
// Created by mrsandman on 09.12.18.
//
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

//Socket interaction result
#define WORKING_SOCKET 0
#define READING_ERROR 401
#define WRITING_ERROR 402
#define READING_IS_NOT_FINISHED 403
#define REQUEST_LENGTH_ERROR 404

#define RESPONSE_ERROR "ERR" // When we got error on handle request
#define RESPONSE_OK "OK"
#define RESPONSE_TOKEN "TOKEN" // When response contains session token
#define RESPONSE_DELETED "DELETED" // When we deleted client data

struct command {
    char* type;
    char* arg1;
};

// Request from client
struct request {
    struct command comm;
    char* token;
};

// Function for creating listen socket
// Returns socket descriptor
int listen_socket();

// Close socket for read and write and print message
void close_socket(int sockfd, char* msg);

// Get request from client
int get_request(int sockfd, struct request* req);

// Read data from socket
int read_socket(int sockfd, char* buffer, int length);

// Send responce to client
// Wrap response to byte array and write it to socket
int response_request(int sockfd, char* type, char* payload);