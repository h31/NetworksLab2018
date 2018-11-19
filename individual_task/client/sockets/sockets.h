#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#define OK 0
#define INVALID_TYPE 100
#define ERROR_READING_FROM_SOCKET 101
#define READING_IS_NOT_FINISHED 102
#define REQUEST_LENGTH_ERROR 103
#define ERROR_WRITING_TO_SOCKET 104

// Define response types
#define RESPONSE_ERROR "ERR" // When server got error on handle request
#define RESPONSE_OK                                                            \
  "OK" // When all is ok and response contains information about bank account
#define RESPONSE_TOKEN "TOKEN"     // When response contains session token
#define RESPONSE_DELETED "DELETED" // When server deleted our data

// Command
struct command {
  char *type;
  char *arg1;
  char *arg2;
};

// Request to server
struct request {
  struct command comm;
  char *token;
};

// Response from server
struct response {
  char *type;
  char *payload;
};

// Function for connecting to server
int connect_to(int argc, char **argv);

// Close socket for read and write and print message
void close_socket(int sockfd, char *msg);

// Wrap request into byte array and send to client
int send_request(int sockfd, struct request *req);

// Create request from string str
int parse_request(char *str, struct request *req);

int get_response(int sockfd, struct response *resp);

int read_from(int sockfd, char *buffer, int length);
