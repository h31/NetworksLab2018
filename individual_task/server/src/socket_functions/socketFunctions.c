#include "socketFunctions.h"

/**This function is for reading client's message from socket
 * param: int sock - socket through which the connection to the client is established
 *        pthread_t pthread - thread in which the client is processed
 * return: char* buffer - message from the client*/
char *readMessage(int sock, pthread_t pthread) {
    writeNextOperation(sock, true, 0);

    char* buffer = (char*)calloc(MAX_BUFFER, sizeof(char));
    char* bufForLen = (char*)calloc(4, sizeof(char));
    uint32_t messlen;


    ssize_t n = read(sock, bufForLen, 4);

    if (n < 0) {
        perror("ERROR lenght of message");
        closeSocket(sock);
        pthread_exit(pthread);
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
            pthread_exit(pthread);
        }
    }

    buffer[strcspn(buffer, "\n")] = 0;
    return buffer;

}

/**This function is for writing a response to the client
 * param: int sock - socket through which the connection to the client is established
 *        char* buffer - server's message
 *        pthread_t pthread - thread in which the client is processed
 * return: void*/
void writeMessage(int sock, char *buffer, pthread_t pthread) {
    writeNextOperation(sock, false, 0);

    char* bufForLen = (char*)calloc(4, sizeof(char));
    uint32_t messlen = strlen(buffer);

    sprintf(bufForLen, "%04d", messlen);
    ssize_t n = write(sock, bufForLen, strlen(bufForLen));

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(sock);
        pthread_exit(pthread);
    }
    free(bufForLen);

    n = write(sock, buffer, messlen);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sock);
        pthread_exit(pthread);
    }
}

/**Here is function for closing socket
 * param: int sock - socket which we want to close
 * return: void */
void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}

/**This is an auxiliary function for working with a client
 * If server sends 1 = client need to write something
 * If server sends 0 = client need to read server's response
 * param: int sock - socket through which the connection to the client is established
 *        bool isRead - true - server send 1, false - send 0
 *        pthread_t pthread - thread in which the client is processed
 * return; void */
void writeNextOperation(int sock, bool isRead, pthread_t pthread) {
    char *nextOp = malloc(sizeof(char));
    nextOp[0] = (char) (isRead == true ? '1' : '0');


    char* bufForLen = (char*)calloc(4, sizeof(char));
    sprintf(bufForLen, "%04d", 1);
    ssize_t n = write(sock, bufForLen, strlen(bufForLen));

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(sock);
        pthread_exit(pthread);
    }

    free(bufForLen);

    n = write(sock, nextOp, 1);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sock);
        pthread_exit(pthread);
    }
}