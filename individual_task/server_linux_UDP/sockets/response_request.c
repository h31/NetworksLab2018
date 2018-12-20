//
// Created by mrsandman on 09.12.18.
//
#include "socket.h"

int response_request(int sockfd, char* type, char* payload, struct sockaddr_in * cli_addr, int index)
{
    int type_length; // Length of response type
    int payload_length; // Length of payload type
    int length; // Total length of response
    char* buf; // Buffer for response

    int res;
    int buf_pointer = 0;

    res = sendto(sockfd, &index, sizeof(int), 0, (struct sockaddr *) cli_addr, sizeof(*cli_addr));
    if (res == -1) {
        close_socket(sockfd, "ERROR write length to socket");
        return -1;
    }

    // Set type length
    type_length = strlen(type);

    // Set payload length
    payload_length = strlen(payload);

    // Count total length of response
    length = sizeof(int) * 2 + (type_length + payload_length) * sizeof(char);

    // Send length to client
    res = sendto(sockfd, &length, sizeof(int), 0, (struct sockaddr *)cli_addr, sizeof(*cli_addr));
    if (res < 0) {
        return WRITING_ERROR;
    }

    // Allocate memory
    buf = (char*)malloc(length);
    bzero(buf, length);

    // Set response type length
    bcopy(&type_length, &buf[buf_pointer], sizeof(int));
    buf_pointer += sizeof(int);

    // Set response type
    bcopy(type, &buf[buf_pointer], type_length * sizeof(char));
    buf_pointer += type_length * sizeof(char);

    // Set response payload length
    bcopy(&payload_length, &buf[buf_pointer], sizeof(int));
    buf_pointer += sizeof(int);

    // Set response payload
    bcopy(payload, &buf[buf_pointer], payload_length * sizeof(char));
    buf_pointer += payload_length * sizeof(char);

    // Send response to client
    res = sendto(sockfd, buf, length, 0, (struct sockaddr *) cli_addr, sizeof(*cli_addr));
    free(buf);

    if (res < 0) {
        return WRITING_ERROR;
    }

    return WORKING_SOCKET;
}
