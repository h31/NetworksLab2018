#include "input.h"
#include "line.h"
#include "number.h"
#include <stdio.h>
#include <stdlib.h>

#define ERR (-1)
#define PTR_ERR NULL

// return: 1 if the user typed number;
//         0 if the user typed not a number;
//         -1 on error
int inputDouble(const char *prompt, double *val) {
    char *line;
    int res;

    printf("%s", prompt);
    line = getLine();
    if (line == PTR_ERR) {
        return ERR;
    }
    res = strToDFinite(line, val);
    free(line);
    return res == ERR ? 0 : 1;
}

// return: -1 on error
//         0 on success
int tryInputDouble(const char *prompt, double *val) {
    int correctNumber;

    while ((correctNumber = inputDouble(prompt, val)) != ERR) {
        if (correctNumber) {
            return 0;
        }
        puts("You typed not a number; please, try to type the number");
        continue;
    }
    return ERR;
}

