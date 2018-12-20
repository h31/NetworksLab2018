//
// Created by mrsandman on 09.12.18.
//

#include "socket.h"

int parse_request(char* str, struct request* req)
{
    char* str_token; // First found token in the string str

    // Get type of request
    str_token = strtok(str, " \n");
    if (str_token == NULL)
        return INVALID_REQUEST;
    req->comm.type = str_token;

    // Get arg1
    str_token = strtok(NULL, " \n");
    if (str_token == NULL) {
        req->comm.arg1 = 0;
        return WORKING_SOCKET;
    }
    req->comm.arg1 = str_token;

    return WORKING_SOCKET;
}