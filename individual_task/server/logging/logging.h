#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#define mlog(msg) internal_log(__func__, msg)
#define mlogf(msg, ...) internal_log_formatted(__func__, msg, __VA_ARGS__)

extern int console_log;
extern FILE* file_to_log;

void internal_log(char const* caller_func, char* msg);

void internal_log_formatted(char const* caller_func, const char* msg, ...);

void log_init(char* filename);

void log_close();
