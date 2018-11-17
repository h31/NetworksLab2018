#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>

#define mlog(msg) internal_log(__func__, msg)
#define mlogf(msg, ...) internal_log_formatted(__func__, msg, __VA_ARGS__)

extern int console_log;

void internal_log(char const * caller_func, char* msg);

void internal_log_formatted(char const * caller_func, const char* msg, ...);
