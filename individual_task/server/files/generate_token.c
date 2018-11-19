#include "files.h"

static const char alphanum[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

void generate_token(char *token, int length) {
  struct timeval tval; // Current milliseconds for random seed
  gettimeofday(&tval, NULL);
  srand(tval.tv_usec);
  int i;
  for (int i = 0; i < length; i++) {
    token[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }
  token[length] = 0;
}
