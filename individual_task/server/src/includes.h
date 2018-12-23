#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>


#include "structures.h"
#include "file_manager/fileManager.h"
#include "socket_functions/socketFunctions.h"
#include "menu_listener/optionsMenuListener.h"
#include "client_actions/commonActions.h"
#include "client_actions/testerActions.h"
#include "client_actions/developerActions.h"
#include "server_actions/serverThreads.h"
#include "server_actions/serverActions.h"

#define MAX_BUFFER 256
#define MAX_REPO 100
#define N 4

extern int bugs_number;
extern long bugId;
extern struct repository_data *rep_data;
extern struct client_data *client_array[N];
extern int sockfd;
extern bool run;
