//
// Created by mrsandman on 09.12.18.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#define WORKING_SOCKET 0
#define INVALID_REQUEST 400
#define READING_ERROR 401
#define READING_IS_NOT_FINISHED 403
#define REQUEST_LENGTH_ERROR 404

// Define response types
#define RESPONSE_ERROR "ERR" // When server got error on handle request
#define RESPONSE_OK "OK"
#define RESPONSE_TOKEN "TOKEN" // When response contains session token
#define RESPONSE_DELETED "DELETED" // When server deleted our data

// Command
struct command {
    char* type;
    char* arg1;
    char* arg2;
};

// Request to server
struct request {
    struct command comm;
    char* token;
};

// Response from server
struct response {
    char* type;
    char* payload;
};

// Function for connecting to server
int connect_socket(struct sockaddr_in* serv_addr);

// Close socket for read and write and print message
void close_socket(int sockfd, char* msg);

// Wrap request into byte array and send to client
int send_request(int sockfd, struct request* req, struct sockaddr_in * serv_addr);

// Create request from string str
int parse_request(char* str, struct request* req);

// Get response from server
int response(int sockfd, struct response* resp, struct sockaddr_in * serv_addr);

// Read from socket
int read_socket(int sockfd, char* buffer, int length, struct sockaddr_in * serv_addr);
