#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "response_applier.h"
#include "cmd_common.h"

#define ERR (-1)
#define PTR_ERR NULL

// create response structure from datagram with server-response
static void *materializeResponse(const void *responseGram, int responseGramLen);

// display result of the response
static void displayResult(const response_t *response);

// return -1 on error; otherwise, 0
int applyResponse(const void *responseGram, int responseGramLen) {
    response_t *response;

    response = materializeResponse(responseGram, responseGramLen);
    if (!response) {
        return ERR;
    }
    displayResult(response);
    free(response);
    return 0;
}

static void *materializeResponse(const void *responseGram, int responseGramLen) {
    response_t* response;

    if (responseGramLen != RESPONSE_LEN) {
        puts("[error] datagram with server-response has unappropriate length");
        printf("Expected %d bytes, actually - %d bytes\n", (int) RESPONSE_LEN, responseGramLen);
        return PTR_ERR;
    }
    response = (response_t *) malloc(sizeof(response_t));
    if (!response) {
        perror("materializeResponse()");
        return PTR_ERR;
    }
    memcpy(&response->cmd, responseGram, CMD_LEN);
    memcpy(&response->op1, responseGram+CMD_LEN, OP_LEN);
    memcpy(&response->op2, responseGram+CMD_LEN+OP_LEN, OP_LEN);
    memcpy(&response->answer, responseGram+CMD_LEN+(OP_LEN<<1), OP_LEN);
    return response;
}

// display result of the response
static void displayResult(const response_t *response) {
    char opSymbol;

    if (response->cmd >= CMD_ADD && response->cmd <= CMD_DIV) {
        if (response->cmd == CMD_ADD) {
            opSymbol = '+';
        } else if (response->cmd == CMD_SUB) {
            opSymbol = '-';
        } else if (response->cmd == CMD_MUL) {
            opSymbol = '*';
        } else if (response->cmd == CMD_DIV) {
            opSymbol = '/';
        }
        printf("%g %c %g = %g\n", response->op1, opSymbol, response->op2, response->answer);
    } else if (response->cmd == CMD_FACT) {
        printf("\n%g! = %g\nYou may enter another operation: ", response->op1, response->answer);
    } else if (response->cmd == CMD_SQRT) {
        printf("\nsqrt(%g) = %g\nYou may enter another operation: ", response->op1, response->answer);
    } else {
        puts("Server answered with unknown command; sorry");
    }
    fflush(stdout);
}


