#include "sockets.h"

int parse_request(char* str, struct request* req)
{
    char* str_token; // First found token in the string str

    // Get type of request
    str_token = strtok(str, " \n");
    if (str_token == NULL)
        return INVALID_TYPE;
    req->comm.type = str_token;

    // Get arg1
    str_token = strtok(NULL, " \n");
    if (str_token == NULL) {
        req->comm.arg1 = 0;
        req->comm.arg2 = 0;
        return OK;
    }
    req->comm.arg1 = str_token;

    // Get arg2
    str_token = strtok(NULL, " \n");
    if (str_token == NULL) {
        req->comm.arg2 = 0;
        return OK;
    }
    req->comm.arg2 = str_token;
    return OK;
}
