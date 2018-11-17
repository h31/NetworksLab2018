#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define mylog(msg) internal_log(__func__, msg)

void internal_log(char const * caller_func, char* msg);
