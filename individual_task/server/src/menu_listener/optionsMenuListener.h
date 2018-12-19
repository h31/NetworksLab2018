#ifndef BUGTRACKER_OPTIONSMENULISTENER_H
#define BUGTRACKER_OPTIONSMENULISTENER_H

#include "../includes.h"

int testerMenuSwitcher(struct client_data *data);
int developerMenuSwitcher(struct client_data *data);
int optionsMenuListener(int position, struct client_data *data);
void printMenu(int position, struct client_data *client_data);

#endif //BUGTRACKER_OPTIONSMENULISTENER_H
