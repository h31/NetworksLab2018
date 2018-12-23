#include "developerActions.h"

/**Here is main action of the developer - mark active bug as fixed
 * param: struct client_data - structure with information about each user
 * return: void*/
void markBugAsFixed(struct client_data *data) {
    writeMessage(data->newsockfd, "Enter bug id\n", 0);

    long id;
    int index;
    struct repository_data *bug = malloc(sizeof(struct repository_data));

    while ((id = strtol(readMessage(data->newsockfd, 0), NULL, 10)) == 0) {
        writeMessage(data->newsockfd, "Only digits are allowed! Please, try again\n", data->pthread);
    }
    if ((index = contains(id, data)) < 0) {
        writeMessage(data->newsockfd, "There is no such bug with given id! Please, try again\n", data->pthread);
        return;
    }

    rep_data[index].bugStatus = "fixed";
    writeMessage(data->newsockfd, "Operation succeed\n", data->pthread);
}