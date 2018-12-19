#ifndef BUGTRACKER_READMESSAGE_H
#define BUGTRACKER_READMESSAGE_H

#include "includes.h"

char* readMessage(int sock);
void writeMessage(int sock, char *buffer);
void closeSocket (int sock);

#endif //BUGTRACKER_READMESSAGE_H
