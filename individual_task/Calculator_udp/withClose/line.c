#include "line.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_BUFFER 80
#define PTR_ERR NULL

// similar to getLineLN(), but ended '\n'-symbol always
// removed from returned buffer
char *getLine() {
    char *res = getLineNL();
    int len = res ? strlen(res) : 0;

    if (len && res[len - 1] == '\n') {
        res[len - 1] = '\0';
    }
    return res;
}

// returns line from keyboard (with memory auto-allocation)
// or NULL if there was some error: input or memory.
// line can be ended not with '\n' if it is the line before the EOF
// if getLineNL() returns not NULL value, you should use free(res)
// to release memory after using your line.
//
// If this function returns NULL (input or memory error) it is considered
// that you don't want to work with user after that.
// But you can still invoke this function if needed.
// Do we need extra code to test what error exactly happened? often we do not.
//
// Also, if some '\0'-symbols entered from keyboard, this can lead to
// missing some entered symbols, but it is ok
// NL means that NewLine character will not be discarded from the buffer.
char *getLineNL() {
    char *res = PTR_ERR, *tmpRes;
    char fgetsRes[MAX_BUFFER];
    int len, lenAll = 0;

    while (fgets(fgetsRes, MAX_BUFFER, stdin)) {
        len = strlen(fgetsRes);
        tmpRes = realloc(res, lenAll + len + 1);
        if (!tmpRes) {
            if (res) free(res);
            return PTR_ERR;
        }
        res = tmpRes;
        memcpy(res + lenAll, fgetsRes, len + 1);
        lenAll += len;
        if (len && fgetsRes[len - 1] == '\n') {
            return res;
        }
    }
    if (feof(stdin)) {
        return res;
    }
    if (res) free(res);
    return PTR_ERR;
}

