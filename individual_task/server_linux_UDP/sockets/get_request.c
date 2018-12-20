//
// Created by mrsandman on 09.12.18.
//
#include "socket.h"

int get_request(int sockfd, struct request* req, struct sockaddr_in * cli_addr)
{
    int readRes; // Result of reading
    char* buf; // Buffer for reading
    int message_length; // Length of message without first sizeof(int) bytes
    int num; // Number of message

    int arg_length;
    int buf_pointer = 0;


    // Get index of message
    buf = (char*)malloc(sizeof(int));
    readRes = read_socket(sockfd, buf, sizeof(int), cli_addr);
    // Throw errors
    if (readRes != WORKING_SOCKET) {
        return readRes;
    }

    num = *(int*)buf;
    if (num == req->index) {
        printf("Lost or wrong packet");
        return LOST_OR_WRONG_PACKET;
    }
    req->index = num;

    // Clear memory
    free(buf);

    // Get length of request
    buf = (char*)malloc(sizeof(int));
    readRes = read_socket(sockfd, buf, sizeof(int), cli_addr);

    if (readRes != WORKING_SOCKET) {
        return readRes;
    }

    message_length = *(int*)buf;
    if (message_length <= 0 || message_length > 65536) {
        return REQUEST_LENGTH_ERROR;
    }

    // Clear memory
    free(buf);

    // Read byte array of request
    buf = (char*)malloc(message_length * sizeof(char));

    readRes = read_socket(sockfd, buf, message_length, cli_addr);

    if (readRes != WORKING_SOCKET) {
        return readRes;
    }

    // Now convert byte array to request struct
    // Get length of type
    bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
    buf_pointer += sizeof(int);

    // Get type of request
    req->comm.type = (char*)malloc(arg_length * sizeof(char) + 1);
    bzero(req->comm.type, arg_length * sizeof(char) + 1);
    bcopy(&buf[buf_pointer], req->comm.type, arg_length * sizeof(char));
    buf_pointer += arg_length * sizeof(char);

    // Get length of arg1
    bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
    buf_pointer += sizeof(int);

    // Get arg1
    if (arg_length > 0) {
        req->comm.arg1 = (char*)malloc(arg_length * sizeof(char) + 1);
        bzero(req->comm.arg1, arg_length * sizeof(char) + 1);
        bcopy(&buf[buf_pointer], req->comm.arg1, arg_length * sizeof(char));
        buf_pointer += arg_length;
    } else {
        req->comm.arg1 = 0;
    }

    // Get token
    if (message_length - buf_pointer == 0) {
        req->token = 0;
    } else {
        arg_length = message_length - buf_pointer;
        req->token = (char*)malloc(arg_length * sizeof(char) + 1);
        bzero(req->token, arg_length * sizeof(char) + 1);
        bcopy(&buf[buf_pointer], req->token, arg_length * sizeof(char));
    }

    // Clear memory
    free(buf);

    return WORKING_SOCKET;
}
