#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "request.h"
#include "line.h"
#include "input.h"
#include "cmd_common.h"

#define ERR (-1)
#define PTR_ERR NULL

// get command from terminal or -1 on error
static cmd_t getCmd();

// create datagram from command, two operands and set length of
// it in gramLen
// return NULL on error; otherwise on the heap allocated datagram
static void *makeRequest(cmd_t cmd, double op1, double op2, int *requestLen);

// return: NULL if it is the last datagram for sending
// otherwise, return pointer to datagram for sending
void *getRequest(int *gramLen) {
    cmd_t cmd;
    double op1, op2;
    int retVal;

    while (1) {
        cmd = getCmd();
        if (cmd == -1 || cmd == CMD_QUIT) {
            return PTR_ERR;
        }
        if (cmd == CMD_UNKNOWN) {
            puts("Unknown command");
            puts("Please, use one of the following: +, -, *, /, !, sqrt, quit");
            continue;
        }
        if (cmd >= CMD_ADD && cmd <= CMD_DIV) {
            if (tryInputDouble("Operand 1: ", &op1) == ERR) {
                return PTR_ERR;
            }
            while ((retVal = tryInputDouble("Operand 2: ", &op2)) != ERR) {
                if (cmd == CMD_DIV && op2 == 0.) {
                    puts("Division to zero is bad; please, enter another number");
                    continue;
                }
                break;
            }
            if (retVal == ERR) {
                return PTR_ERR;
            }
        } else if (cmd >= CMD_FACT && cmd <= CMD_SQRT) {
            if (tryInputDouble("Operand: ", &op1) == ERR) {
                return PTR_ERR;
            }
        }
        return makeRequest(cmd, op1, op2, gramLen);
    }
}

static void *makeRequest(cmd_t cmd, double op1, double op2, int *requestLen) {
    void *res;

    if (!(res = malloc(REQUEST_LEN))) {
        return PTR_ERR;
    }
    *requestLen = REQUEST_LEN;
    memcpy(res, &cmd, CMD_LEN);
    memcpy(res+CMD_LEN, &op1, OP_LEN);
    memcpy(res+CMD_LEN+OP_LEN, &op2, OP_LEN);
    return res;
}

static cmd_t getCmd() {
    char *line;
    cmd_t res = CMD_UNKNOWN;

    printf("Input command (+, -, *, /, !, sqrt, quit): ");
    line = getLine();
    if (line == NULL) {
        return ERR;
    }
    if (!strcmp(line, "+")) {
        res = CMD_ADD;
    } else if (!strcmp(line, "-")) {
        res = CMD_SUB;
    } else if (!strcmp(line, "*")) {
        res = CMD_MUL;
    } else if (!strcmp(line, "/")) {
        res = CMD_DIV;
    } else if (!strcmp(line, "!")) {
        res = CMD_FACT;
    } else if (!strcmp(line, "sqrt")) {
        res = CMD_SQRT;
    } else if (!strcmp(line, "quit")) {
        res = CMD_QUIT;
    }
    free(line);
    return res;
}

