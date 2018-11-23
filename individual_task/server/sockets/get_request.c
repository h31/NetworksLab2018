#include "sockets.h"

int get_request(int sockfd, struct request* req)
{
    int res; // Result of reading
    char* buf; // Buffer for reqding
    int message_length; // Length of message without first sizeof(int) bytes

    int arg_length;
    int buf_pointer = 0;

    // Get length of request
    buf = (char*)malloc(sizeof(int));
    res = read_from(sockfd, buf, sizeof(int));
    // Throw errors
    if (res != SOCKETS_OK) {
        return res;
    }

    message_length = *(int*)buf;
    if (message_length <= 0 || message_length > 256) {
        mlogf("Illegal length of request: %d", message_length);
        return REQUEST_LENGTH_ERROR;
    }
    mlogf("Request length = %d", message_length);

    // Clear memory
    free(buf);

    // Read byte array of request
    buf = (char*)malloc(message_length * sizeof(char));

    res = read_from(sockfd, buf, message_length);

    // Throw errors
    if (res != SOCKETS_OK) {
        return res;
    }
    mlog("Request was read");

    // Now convert byte array to request struct
    // Get length of type
    bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
    buf_pointer += sizeof(int);

    mlogf("type length = %d", arg_length);

    // Get type of request
    req->comm.type = (char*)malloc(arg_length * sizeof(char) + 1);
    bzero(req->comm.type, arg_length * sizeof(char) + 1);
    bcopy(&buf[buf_pointer], req->comm.type, arg_length * sizeof(char));
    buf_pointer += arg_length * sizeof(char);

    mlogf("type = %s", req->comm.type);

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

    // Get length of arg2
    bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
    buf_pointer += sizeof(int);

    // Get arg2
    if (arg_length > 0) {
        req->comm.arg2 = (char*)malloc(arg_length * sizeof(char) + 1);
        bzero(req->comm.arg2, arg_length * sizeof(char) + 1);
        bcopy(&buf[buf_pointer], req->comm.arg2, arg_length * sizeof(char));
        buf_pointer += arg_length;
    } else {
        req->comm.arg2 = 0;
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

    mlog("Request is parsed");

    // Clear memory
    free(buf);

    return SOCKETS_OK;
}
