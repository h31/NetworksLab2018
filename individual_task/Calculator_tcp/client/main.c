#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <strings.h>
#include <errno.h>

#define MAX_SIZE 50

int getSO_ERROR(int fd) {
    int err = 1;
    socklen_t len = sizeof err;
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len)) {
        perror("getSO_ERROR");
        exit(EXIT_FAILURE);
    }
    if (err) {
        errno = err;
    }
    return err;
}

void closeSocket(int fd) {
    if (fd > 0) {
        getSO_ERROR(fd);
        if (shutdown(fd, SHUT_RDWR) < 0) {
            if (errno != ENOTCONN && errno != EINVAL) {
                perror("shutdown");
                exit(EXIT_FAILURE);
            }
        }
        if (close(fd) < 0) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}


enum Command {
    CMD_UNKNOWN = 0,
    CMD_QUIT,
    CMD_ADD,
    CMD_SUB,
    CMD_MUL,
    CMD_DIV,
    CMD_FACT,
    CMD_SQRT,
    CMD_HEARTBIT,
    CMD_SETID
};

char opChars[] = {
    '+', '-', '*', '/'
};

char getQuickOpChar(enum Command cmd) {
    return opChars[((int)cmd) - 2];
}

enum Command defineCommand(const char* cmdStr) {
    while (*cmdStr != '\0' && *cmdStr == ' ') {
        cmdStr++;
    }
    enum Command res;
    if (!strcmp(cmdStr, "quit\n")) {
        res = CMD_QUIT;
    } else if (!strcmp(cmdStr, "+\n")) {
        res = CMD_ADD;
    } else if (!strcmp(cmdStr, "-\n")) {
        res = CMD_SUB;
    } else if (!strcmp(cmdStr, "*\n")) {
        res = CMD_MUL;
    } else if (!strcmp(cmdStr, "/\n")) {
        res = CMD_DIV;
    } else if (!strcmp(cmdStr, "!\n")) {
        res = CMD_FACT;
    } else if (!strcmp(cmdStr, "sqrt\n")) {
        res = CMD_SQRT;
    } else if (!strcmp(cmdStr, "check\n")) {
        res = CMD_HEARTBIT;
    }
    if (res == CMD_UNKNOWN) {
        //printf("cmd = !%s!\n", cmdStr);
    }
    return res;
}

double tryInputDouble(const char* prompt) {
    double number;
    char inputLine[MAX_SIZE];
    int success;
    do {
        printf("%s", prompt);
        fgets(inputLine, MAX_SIZE, stdin);
        success = sscanf(inputLine, " %lf ", &number);
        if (!success) {
            printf("You entered not a number. Please, try again\n");
        }
    } while (!success);
    return number;
}

bool sendSimpleCommand(int sock, enum Command cmd) {
    uint32_t cmdAsInt = (uint32_t) cmd;
    unsigned wereSentCnt = send(sock, &cmdAsInt, sizeof(uint32_t), 0);
    return wereSentCnt == sizeof(uint32_t);
}

// returns true if success
bool sendQuickOpInput(int sock, enum Command cmd, double op1, double op2, double* result) {
    bool success;
    unsigned len = sizeof(int) + (sizeof(double)<<1);
    char* allInfo = (char*)malloc(len);
    uint32_t cmdAsInt = (uint32_t) cmd;
    memcpy(allInfo, &cmdAsInt, sizeof(uint32_t));
    memcpy(allInfo + sizeof(uint32_t), &op1, sizeof(double));
    memcpy(allInfo + sizeof(uint32_t) + sizeof(double), &op2, sizeof(double));
    unsigned wereSentCnt = send(sock, allInfo, len, 0);
    free(allInfo);
    success = wereSentCnt == len;
    if (!success) {
        return false;
    }
    unsigned receivedCnt = recv(sock, result, sizeof(double), 0);
    success = receivedCnt == sizeof(double);
    return success;
}

// returns true if success
bool sendLongOpInput(int sock, enum Command cmd, double op) {
    unsigned len = sizeof(int) + sizeof(double);
    char* allInfo = (char*)malloc(len);
    uint32_t cmdAsInt = (uint32_t) cmd;
    memcpy(allInfo, &cmdAsInt, sizeof(uint32_t));
    memcpy(allInfo + sizeof(uint32_t), &op, sizeof(double));
    unsigned wereSentCnt = send(sock, allInfo, len, 0);
    free(allInfo);
    return wereSentCnt == len;
}

// if return value is true, then it is the last command
// otherwise, not the last command
bool processCommand(int sock, enum Command cmd, bool *heartBitContinue) {
    *heartBitContinue = false;
    if (cmd == CMD_UNKNOWN) {
        printf("Unknown command\n");
    } else if (cmd == CMD_QUIT) {
        sendSimpleCommand(sock, cmd);
        printf("Bye\n");
        return true;
    } else if (cmd == CMD_ADD || cmd == CMD_SUB || cmd == CMD_MUL || cmd == CMD_DIV) {
        bool success;
        // quick operation algorithm
        double op1, op2, result;
        op1 = tryInputDouble("First operand: ");
        op2 = tryInputDouble("Second operand: ");
        success = sendQuickOpInput(sock, cmd, op1, op2, &result);
        if (!success) {
            printf("Failed to get the result\nBye\n");
            return true;
        }
        printf("%g %c %g = %g\n", op1, getQuickOpChar(cmd), op2, result);
    } else if (cmd == CMD_FACT || cmd == CMD_SQRT) {
        // long operation algorithm
        double op;
        bool success;
        op = tryInputDouble("Operand: ");
        success = sendLongOpInput(sock, cmd, op);
        if (!success) {
            printf("Failed to get the result\nBye\n");
            return true;
        }
        printf("Command was sent to server\n");
        printf("The result will be received after few seconds\n");
        printf("You can use other commands while waiting\n");
    } else if (cmd == CMD_HEARTBIT) {
        uint32_t cmdAsInt = (uint32_t) cmd;
        if (send(sock, &cmdAsInt, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
            return true;
        }
        uint32_t cmdResAsInt;
        if (recv(sock, &cmdResAsInt, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
            return true;
        }
        if (cmdResAsInt == CMD_FACT || cmdResAsInt == CMD_SQRT) {
            double op, res;
            int timeElapsed;
            if (recv(sock, &op, sizeof(double), 0) != sizeof(double)) {
                return true;
            }
            if (recv(sock, &res, sizeof(double), 0) != sizeof(double)) {
                return true;
            }
            if (recv(sock, &timeElapsed, sizeof(int), 0) != sizeof(int)) {
                return true;
            }
            if (cmdResAsInt == CMD_FACT) {
                printf("\nLong operation result (processing time = %ds): %g! = %g\n", timeElapsed, op, res);
            } else {
                printf("\nLong operation result (processing time = %ds): sqrt(%g) = %g\n", timeElapsed, op, res);
            }
        } else if (cmdResAsInt == CMD_HEARTBIT) {
            *heartBitContinue = true;
            // nothing
            // puts("No long operations were resolved");
        }
    }
    return false;
}

char *clientID;

int main(int argc, char **argv)
{
    const char* hostIP;
    const char* portStr;
    if (argc == 2) {
        clientID = argv[1];
        hostIP = "127.0.0.1";
        portStr = "7000";
    } else if (argc == 4) {
        clientID = argv[1];
        hostIP = argv[2];
        portStr = argv[3];
    } else {
        printf("Use \"%s <client_id> <host_ip> <host_port>\"\n", argv[0]);
        printf("  or \"%s <client_id>\"\n", argv[0]);
        return 1;
    }
    if (strlen(clientID) > 49) {
        printf("Client ID should be less than 50 characters long\n");
        return EXIT_FAILURE;
    }

    fd_set set;
    struct timeval timeout;
    int rv;
    int sock_desc;
    struct sockaddr_in serv_addr;
    char cmd[MAX_SIZE];//,answer[MAX_SIZE];

    if((sock_desc = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failed creating socket\n");
        return 1;
    }

    bzero((char *) &serv_addr, sizeof (serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(hostIP);
    serv_addr.sin_port = htons(atoi(portStr));

    if (connect(sock_desc, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) < 0) {
        printf("Failed to connect to server\n");
        return 1;
    }

    uint32_t cmdSetID = CMD_SETID;
    uint32_t clientIDLen = strlen(clientID);
    if (send(sock_desc, &cmdSetID, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
        perror("send1()");
        return 1;
    }
    if (send(sock_desc, &clientIDLen, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
        perror("send2()");
        return 1;
    }
    if (send(sock_desc, clientID, clientIDLen, 0) != clientIDLen) {
        perror("send3()");
        return 1;
    }
    printf("Connected successfully client:\n");
    enum Command command = CMD_UNKNOWN;
    bool lastCommand;
    bool heartBitContinue = false;
    while(1)
    {
        FD_ZERO(&set); /* clear the set */
        FD_SET(STDIN_FILENO, &set); /* add our file descriptor to the set */
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        if (!heartBitContinue) {
            printf("Enter command (+,-,*,/,!,sqrt): ");
            fflush(stdout);
        }
        rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
        //printf("%d %d\n", rv, (int)(FD_ISSET(STDIN_FILENO, &set)));
        //printf("%d\n", rand());
        if(rv == -1) {
            // error occured
            break;
        } else if(rv == 0) {
            /* a timeout occured */
            //printf("hi2\n");
            command = CMD_HEARTBIT;
        } else {
            /* there was data to read */
            //printf("hi\n");
            fgets(cmd, MAX_SIZE, stdin);
            command = defineCommand(cmd);
        }
        lastCommand = processCommand(sock_desc, command, &heartBitContinue);
        if (lastCommand) {
            break;
        }
    }
    closeSocket(sock_desc);
    return 0;
}

