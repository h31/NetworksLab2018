//
// Created by mrsandman on 09.12.18.
//

#include "socket.h"

int response(int sockfd, struct response* resp, struct sockaddr_in * serv_addr, struct request* req)
{
    int res; // Result of reading
    char* buf; // Buffer for reading
    int message_length; // Length of message without first sizeof(int) bytes
    int num; // Number of message

    int arg_length;
    int buf_pointer = 0;

    // Get index of message
    buf = (char*)malloc(sizeof(int));
    res = read_socket(sockfd, buf, sizeof(int), serv_addr);
    // Throw errors
    if (res != WORKING_SOCKET) {
        return res;
    }

    num = *(int*)buf;
    if (num != req->index) {
        return LOST_OR_WRONG_PACKET;
    }
    resp->index = num;

    // Clear memory
    free(buf);

    // Get length of request
    buf = (char*)malloc(sizeof(int));
    res = read_socket(sockfd, buf, sizeof(int), serv_addr);
    // Throw errors
    if (res != WORKING_SOCKET) {
        return res;
    }

    message_length = *(int*)buf;
    if (message_length <= 0 || message_length > 65536) {
        printf("Illegal length of request: %d\n", message_length);
        return REQUEST_LENGTH_ERROR;
    }

    // Clear memory
    free(buf);

    // Read byte array of request
    buf = (char*)malloc(message_length * sizeof(char));
    bzero(buf, message_length * sizeof(char));

    res = read_socket(sockfd, buf, message_length, serv_addr);

    // Throw errors
    if (res != WORKING_SOCKET) {
        return res;
    }

    // Now convert byte array to response struct
    // Get length of type
    bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
    buf_pointer += sizeof(int);

    // Get type of response
    resp->type = (char*)malloc(arg_length * sizeof(char) + 1);
    bzero(resp->type, arg_length * sizeof(char) + 1);
    bcopy(&buf[buf_pointer], resp->type, arg_length * sizeof(char));
    buf_pointer += arg_length * sizeof(char);

    // Get length of payload
    bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
    buf_pointer += sizeof(int);

    // Get payload
    resp->payload = (char*)malloc(arg_length * sizeof(char) + 1);
    bzero(resp->payload, arg_length * sizeof(char) + 1);
    bcopy(&buf[buf_pointer], resp->payload, arg_length * sizeof(char));
    buf_pointer += arg_length * sizeof(char);

    // Clear memory
    free(buf);

    return WORKING_SOCKET;
}