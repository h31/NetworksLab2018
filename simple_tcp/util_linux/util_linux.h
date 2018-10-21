#ifndef UTIL_LINUX_H
#define UTIL_LINUX_H

#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket(int socks[], int error, char* errorMsg);
char* readAll(int socks[]);
void sendAll(int socks[], char* buffer);

#endif /* UTIL_LINUX_H */
