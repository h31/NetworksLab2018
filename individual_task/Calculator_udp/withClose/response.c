#include <pthread.h>
#include <unistd.h>
#include "response.h"
#include "cmd_common.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"

#define PTR_ERR NULL

typedef struct {
    cmd_t cmd;
    double op1;
    double op2;
} request_t;

extern pthread_mutex_t randLock;

// make struct request from request datagram
static request_t *materializeRequest(const void *requestGram, int requestLen);

// execute request on server
// return NULL if error occured or response
static response_t *processRequest(const request_t *request);

// convert response structure to datagram
// return NULL on error or pointer to datagram on success
static void *dematerializeResponse(const response_t *response, int *responseGramLen);

// calculate factorial
static int fact(int val);

// make datagram with server-response from datagram with client-request
// return: NULL on error or pointer to autoallocated response if success
void *makeResponse(const void *requestGram, int requestLen, int *responseLen) {
    request_t *request;
    response_t *response;
    void *responseGram;

    request = materializeRequest(requestGram, requestLen);
    if (!request) {
        return PTR_ERR;
    }
    response = processRequest(request);
    free(request);
    if (!response) {
        return PTR_ERR;
    }
    responseGram = dematerializeResponse(response, responseLen);
    free(response);
    return responseGram;
}

static request_t *materializeRequest(const void *requestGram, int requestLen) {
    request_t *request;

    if (requestLen != REQUEST_LEN) {
        return PTR_ERR;
    }
    request = (request_t *) malloc(sizeof(request_t));
    if (!request) {
        return PTR_ERR;
    }
    memcpy(&request->cmd, requestGram, CMD_LEN);
    memcpy(&request->op1, requestGram+CMD_LEN, OP_LEN);
    memcpy(&request->op2, requestGram+CMD_LEN+OP_LEN, OP_LEN);
    return request;
}

// execute request on server
// return NULL if error occured or response
static response_t *processRequest(const request_t *request) {
    response_t *response;

    response = (response_t *) malloc(sizeof(response_t));
    if (!response) {
        return PTR_ERR;
    }
    response->cmd = request->cmd;
    response->op1 = request->op1;
    response->op2 = request->op2;
    response->answer = 0.;
    if (request->cmd == CMD_FACT || request->cmd == CMD_SQRT) {
        int randomValue;
        int minSeconds = 1;
        int maxSeconds = 10;
        pthread_mutex_lock(&randLock);
        randomValue = minSeconds + rand()%(maxSeconds - minSeconds + 1);
        pthread_mutex_unlock(&randLock);
        sleep((unsigned) randomValue);
    }
    if (request->cmd == CMD_ADD) {
        response->answer = request->op1 + request->op2;
    } else if (request->cmd == CMD_SUB) {
        response->answer = request->op1 - request->op2;
    } else if (request->cmd == CMD_MUL) {
        response->answer = request->op1 * request->op2;
    } else if (request->cmd == CMD_DIV) {
        response->answer = request->op1 / request->op2;
    } else if (request->cmd == CMD_FACT) {
        response->answer = fact((int) request->op1);
    } else if (request->cmd == CMD_SQRT) {
        response->answer = sqrt(request->op1);
    }
    return response;
}

// convert response structure to datagram
// return NULL on error or pointer to datagram on success
static void *dematerializeResponse(const response_t *response, int *responseGramLen) {
    void *responseGram;

    responseGram = malloc(RESPONSE_LEN);
    if (!responseGram) {
        return PTR_ERR;
    }
    memcpy(responseGram, &response->cmd, CMD_LEN);
    memcpy(responseGram+CMD_LEN, &response->op1, OP_LEN);
    memcpy(responseGram+CMD_LEN+OP_LEN, &response->op2, OP_LEN);
    memcpy(responseGram+CMD_LEN+(OP_LEN<<1), &response->answer, OP_LEN);
    *responseGramLen = RESPONSE_LEN;
    return responseGram;
}

// calculate factorial
static int fact(int val) {
    if (!val || val == 1) {
        return 1;
    }
    return fact(val - 1)*val;
}

