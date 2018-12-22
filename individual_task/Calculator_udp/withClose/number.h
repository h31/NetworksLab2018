#ifndef NUMBER_H
#define NUMBER_H

// converts str to double as if strtod() function;
// but if after the number non-whitespaces characters
// exists return -1 instead of 0
// Also -1 returns on error
int strToD(const char *str, double *val);

// Works like strToD() function, but
// for NaN, and +-infinity values returns -1 also
int strToDFinite(const char *str, double *val);

#endif

