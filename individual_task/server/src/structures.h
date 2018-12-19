#ifndef BUGTRACKER_STRUCTURES_H
#define BUGTRACKER_STRUCTURES_H

#include "includes.h"

/**Structure with all client data
 * This structure is filled as clients connect
 * Structure for each client is added to an array of structures - client_array*/
struct client_data{
    struct sockaddr_in cli_addr;
    int newsockfd;
    int serial_num;
    char* login;
    pthread_t pthread;
};

/**Structure with all repositore=y data
 * This structure is filled from special file, which is updated after each session
 * Structure for each repository is added to an array of structures - rep_data*/
struct repository_data {
    char* dev_login;
    char* test_login;
    char* project;
    long bugId;
    char* bugStatus;
    char* bugDescription;
};

#endif //BUGTRACKER_STRUCTURES_H
