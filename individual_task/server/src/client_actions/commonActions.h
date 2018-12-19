#ifndef BUGTRACKER_COMMONACTIONS_H
#define BUGTRACKER_COMMONACTIONS_H

#include "../includes.h"

void printRepositoryInfo(struct client_data *data, bool printActive);

int contains(long id, struct client_data *client_data);

#endif //BUGTRACKER_COMMONACTIONS_H
