#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include "../logging/logging.h"

// Define result codes of operations
#define SOCKETS_OK 0
#define ERROR_READING_FROM_SOCKET 101
#define READING_IS_NOT_FINISHED 102
#define REQUEST_LENGTH_ERROR 103
#define ERROR_WRITING_TO_SOCKET 104

// Define response types
#define RESPONSE_ERROR "ERR" // When we got error on handle request
#define RESPONSE_OK                                                            \
  "OK" // When all is ok and response contains information about bank account
#define RESPONSE_TOKEN "TOKEN"     // When response contains session token
#define RESPONSE_DELETED "DELETED" // When we deleted client data

// Command from client
struct command {
  char *type;
  char *arg1;
  char *arg2;
};

// Request from client
struct request {
  struct command comm;
  char *token;
};

// Function for creating listen socket
// Returns socket descriptor
int create_listen_socket();

// Close socket for read and write and print message
void close_socket(int sockfd, char *msg);

// Get request from client
int get_request(int sockfd, struct request *req);

// Read data from socket
int read_from(int sockfd, char *buffer, int length);

// Send responce to client
// Wrap response to byte array and write it to socket
int internal_send_response(int sockfd, char *type, char *payload);
