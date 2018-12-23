#ifndef BUGTRACKER_SERVERTHREADS_H
#define BUGTRACKER_SERVERTHREADS_H

#include "../includes.h"

void *communicateThread(void *thread_data);
void *waitKeyThread();

#endif //BUGTRACKER_SERVERTHREADS_H
