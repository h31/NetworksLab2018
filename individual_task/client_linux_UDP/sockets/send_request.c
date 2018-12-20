//
// Created by mrsandman on 09.12.18.
//

#include "socket.h"

int send_request(int sockfd, struct request* req, struct sockaddr_in * serv_addr, int index)
{
    int type_length; // Length of request type
    int arg1_length; // Length of first argument of command
    int token_length; // Length of token
    int length; // Total length of request
    char* buf; // Wrap request into this buffer
    int buf_pointer = 0;
    int res;

    res = sendto(sockfd, &index, sizeof(int), 0, (struct sockaddr *) serv_addr, sizeof(*serv_addr));
    if (res == -1) {
        close_socket(sockfd, "ERROR write length to socket");
        return -1;
    }

    req->index = index;

    // Set length of type
    type_length = strlen(req->comm.type);

    // Set length of arg1
    if (req->comm.arg1 != NULL) {
        arg1_length = strlen(req->comm.arg1);
    } else {
        arg1_length = 0;
    }

    // Set length of token
    if (req->token != NULL) {
        token_length = strlen(req->token);
    } else {
        token_length = 0;
    }

    // Count total length of requestr
    length = sizeof(int) * 3 + (type_length + arg1_length + token_length) * sizeof(char);

    // Send length to server
    res = sendto(sockfd, &length, sizeof(int), 0, (struct sockaddr *) serv_addr, sizeof(*serv_addr));
    if (res == -1) {
        close_socket(sockfd, "ERROR write length to socket");
        return -1;
    }

    // Allocate memory for request
    buf = (char*)malloc(length);
    bzero(buf, length);

    // Set request type length
    bcopy(&type_length, &buf[buf_pointer], sizeof(int));
    buf_pointer += sizeof(int);

    // Set request type
    bcopy(req->comm.type, &buf[buf_pointer], type_length * sizeof(char));
    buf_pointer += type_length * sizeof(char);

    // Set length of arg1
    bcopy(&arg1_length, &buf[buf_pointer], sizeof(int));
    buf_pointer += sizeof(int);

    // Set arg1
    if (arg1_length > 0) {
        bcopy(req->comm.arg1, &buf[buf_pointer], arg1_length * sizeof(char));
        buf_pointer += arg1_length * sizeof(char);
    }

    // Set token
    bcopy(req->token, &buf[buf_pointer], token_length * sizeof(char));

    // Send request to server
    res = sendto(sockfd, buf, length, MSG_CONFIRM, (struct sockaddr *) serv_addr, sizeof(*serv_addr));
    if (res == -1) {
        close_socket(sockfd, "ERROR write request to socket");
        return -1;
    }

    return WORKING_SOCKET;
}