#ifndef BUGTRACKER_READMESSAGE_H
#define BUGTRACKER_READMESSAGE_H

#include "../includes.h"

char *readMessage(int sock, pthread_t pthread);
void writeMessage(int sock, char *buffer, pthread_t pthread);
void closeSocket (int sock);
void writeNextOperation(int sock, bool isRead, pthread_t pthread);

#endif //BUGTRACKER_READMESSAGE_H
