#include "testerActions.h"

/**Here is function for adding new bug to list of active bugs
 * The tester must type the developer’s name, project, and bug description
 * param: struct client_data - structure with information about each user
 * return: void */
void newBug(struct client_data *data) {
    writeMessage(data->newsockfd, "Type developer login\n", data->pthread);
    rep_data[bugs_number].dev_login = readMessage(data->newsockfd, data->pthread);
    writeMessage(data->newsockfd, "Type project name\n", data->pthread);
    rep_data[bugs_number].project = readMessage(data->newsockfd, data->pthread);
    writeMessage(data->newsockfd, "Type bug description\n", data->pthread);
    rep_data[bugs_number].bugDescription = readMessage(data->newsockfd, data->pthread);

    rep_data[bugs_number].test_login = data->login;
    rep_data[bugs_number].bugStatus = "active";
    rep_data[bugs_number].bugId = ++bugId;

    bugs_number++;
}

/**This function for changing status of fixed bug
 * The tester can close bug or reopen it
 * param: struct client_data - structure with information about each user
 * return: void */
void changeStatus(struct client_data *data) {
    writeMessage(data->newsockfd, "Enter bug id\n", data->pthread);
    long id;
    int index;

    while ((id = strtol(readMessage(data->newsockfd, 0), NULL, 10)) == 0) {
        writeMessage(data->newsockfd, "Only digits are allowed! Please, try again\n", data->pthread);
    }
    if ((index = contains(id, data)) < 0) {
        writeMessage(data->newsockfd, "There is no such bug with given id! Please, try again\n", data->pthread);
        return;
    }

    writeMessage(data->newsockfd, "Enter action. 1 - close, 2 - reopen\n", data->pthread);

    long action;
    while (true) {
        while ((action = strtol(readMessage(data->newsockfd, 0), NULL, 10)) == 0) {
            writeMessage(data->newsockfd, "Only digits are allowed! Please, try again\n", data->pthread);
        }
        if (action == 1 || action == 2) {
            break;
        }
        writeMessage(data->newsockfd, "Only 1 and 2 are allowed! Please, try again\n", data->pthread);
    }

    rep_data[index].bugStatus = action == 1 ? "closed" : "active";
    writeMessage(data->newsockfd, "Operation succeed\n", data->pthread);
}
