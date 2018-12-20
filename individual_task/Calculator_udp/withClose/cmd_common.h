#ifndef CMD_COMMON_H
#define CMD_COMMON_H

#include <stdint.h>

#define CMD_LEN (sizeof(cmd_t))
#define OP_LEN (sizeof(double))
#define REQUEST_LEN (CMD_LEN + (OP_LEN << 1))
#define RESPONSE_LEN (CMD_LEN + (OP_LEN*3))

enum Command {
    CMD_UNKNOWN = 0,
    CMD_QUIT,
    CMD_ADD,
    CMD_SUB,
    CMD_MUL,
    CMD_DIV,
    CMD_FACT,
    CMD_SQRT
};

typedef int32_t cmd_t;

typedef struct {
    cmd_t cmd;
    double op1;
    double op2;
    double answer;
} response_t;

#endif

