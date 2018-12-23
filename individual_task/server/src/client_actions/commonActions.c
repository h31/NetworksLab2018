#include "../includes.h"

/**Here is a function for printing repository info from repository_data structure
 * param: struct client_data - structure with information about each user
 *        bool printActive - if true - print list with active bugs, else - list with fixed bugs
 *return: void*/
void printRepositoryInfo(struct client_data *data, bool printActive) {

    char *buffer = (char *) calloc(MAX_BUFFER, sizeof(char));
    bzero(buffer, MAX_BUFFER);

    snprintf(buffer, MAX_BUFFER, "%-10s| %-10s| %-10s| %-7s| %-10s| %-20s\n", "developer", "tester", "project",
             "bug id", "bug status", "bug description");
    writeMessage(data->newsockfd, buffer, data->pthread);

    char *status = printActive ? "active" : "fixed";
    bool empty = true;
    for (int i = 0; i < bugs_number; i++) {
        if ((strcmp(rep_data[i].test_login, data->login) == 0 || strcmp(rep_data[i].dev_login, data->login) == 0) &&
            strcmp(rep_data[i].bugStatus, status) == 0) {
            bzero(buffer, MAX_BUFFER);
            snprintf(buffer, MAX_BUFFER, "%-10s| %-10s| %-10s| %-7ld| %-10s| %-20s\n",
                     rep_data[i].dev_login,
                     rep_data[i].test_login,
                     rep_data[i].project,
                     rep_data[i].bugId,
                     rep_data[i].bugStatus,
                     rep_data[i].bugDescription);
            writeMessage(data->newsockfd, buffer, data->pthread);
            empty = false;
        }
    }

    free(buffer);

    if (empty) {
        if (strcmp(status, "active") == 0) {
            writeMessage(data->newsockfd, "There are no active bugs\n", data->pthread);
        } else {
            writeMessage(data->newsockfd, "There are no fixed bugs\n", data->pthread);
        }
    }

}

/**Function for search and check some data from repository_data
 * param: long id - id of the bug
 *        struct client_data - structure with information about each user
 * return: if the information is correct, returns int i - index of rep_data = repository_data structure array
 * else returns -1*/
int contains(long id, struct client_data *client_data) {
    for (int i = 0; i < bugs_number; i++) {
        if ((strcmp(rep_data[i].test_login, client_data->login) == 0 ||
             strcmp(rep_data[i].dev_login, client_data->login) == 0)
            && rep_data[i].bugId == id) {
            return i;
        }
    }
    return -1;
}
