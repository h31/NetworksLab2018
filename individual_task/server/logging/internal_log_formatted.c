#include "logging.h"

void internal_log_formatted(char const *caller_func, const char *msg, ...) {
  va_list arg;
  int done;
  char buf[256];

  va_start(arg, msg);
  done = vsprintf(buf, msg, arg);
  va_end(arg);
  if (done <= 0) {
    printf("ERROR on formatted log. done = %d\n", done);
    return;
  }
  internal_log(caller_func, buf);
}
