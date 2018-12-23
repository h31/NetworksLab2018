#include "socketFunctions.h"

/**This function is for reading a message from the server
 * param: int sock - socket through which the connection to the server is established
 * return: char* buffer - message from server */
char* readMessage(int sock) {

    char* buffer = (char*)calloc(MAX_BUFFER, sizeof(char));
    char* bufForLen = (char*)calloc(4, sizeof(char));
    uint32_t messlen;

    ssize_t n = read(sock, bufForLen, 4);

    if (n < 0) {
        perror("ERROR lenght of message");
        closeSocket(sock);

        exit(1);
    }

    messlen = atol(bufForLen);
    free(bufForLen);

    int received = 0;

    while(received < messlen) {

        n = read(sock, buffer, messlen - received);
        received +=n;

        if (n < 0) {
            perror("ERROR of message");
            closeSocket(sock);

            exit(1);
        }
    }
    return buffer;
}

/**This function is for writing a message to the server
 * param: int sock - socket through which the connection to the server is established
 *        char* buffer - client's message
 * return: void*/
void writeMessage(int sock, char *buffer) {

    char* bufForLen = (char*)calloc(4, sizeof(char));
    uint32_t messlen = strlen(buffer);

    sprintf(bufForLen, "%04d", messlen);
    ssize_t n = write(sock, bufForLen, strlen(bufForLen));

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(sock);

        exit(1);
    }
    free(bufForLen);

    n = write(sock, buffer, messlen);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sock);

        exit(1);
    }
}

/**Here is function for closing socket
 * param: int sock - socket which we want to close
 * return: void */
void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}