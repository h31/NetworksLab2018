//#include <errno.h>
//#include <stddef.h>
//#include <stdio.h>
#include <stdlib.h>
//#include <stdint.h>
//#include <string.h>
#include <ctype.h>
#include <math.h>

#define ERR -1
#define PTR_ERR NULL

// converts str to double as if strtod() function;
// but if after the number non-whitespaces characters
// exists return -1 instead of 0
// Also -1 returns on error
int strToD(const char *str, double *val) {
    char *endPtr;

    *val = strtod(str, &endPtr);
    if (endPtr == str) {
        return ERR;
    }
    while (*endPtr && isspace(*endPtr)) {
        endPtr++;
    }
    return !*endPtr ? 0 : ERR;
}

// Works like strToD() function, but
// for NaN, and +-infinity values returns -1 also
int strToDFinite(const char *str, double *val) {
    int res;
    
    res = strToD(str, val);
    if (!isfinite(*val)) {
        *val = 0.;
        return ERR;
    }
    return res;
}

