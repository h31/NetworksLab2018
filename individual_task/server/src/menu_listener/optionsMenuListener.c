#include "optionsMenuListener.h"

/**Common function for printing a menu and calling the necessary handlers
 * param: int position - 1 -> user is a tester, 0 -> user is a developer
 *        struct client_data - structure with information about each user
 * return: int - status of each chosen action */
int optionsMenuListener(int position, struct client_data *data) {

    printMenu(position, data);
    switch (position) {
        case 1:
            return testerMenuSwitcher(data);
        case 0:
            return developerMenuSwitcher(data);
        default:
            return -1;
    }
}

/**There is a menu handler for tester
 * param: struct client_data - structure with information about each user
 * return: int - status of each chosen action */
int testerMenuSwitcher(struct client_data *data) {
    writeMessage(data->newsockfd, "Choose an action\n", data->pthread);
    char *menuPointString = readMessage(data->newsockfd, data->pthread);

    long menuPoint = strtol(menuPointString, NULL, 10);

    switch (menuPoint) {
        case 1:
            printRepositoryInfo(data, 0);
            return 1;
        case 2:
            printRepositoryInfo(data, 1);
            return 2;
        case 3:
            newBug(data);
            return 3;
        case 4:
            changeStatus(data);
            return 4;
        case 5:
            closeSocket(data->newsockfd);
            return -1;
        default:
            writeMessage(data->newsockfd, "Error choosing menu\n", data->pthread);
            return 0;
    }
}

/**There is a menu handler for developer
 * param: struct client_data - structure with information about each user
 * return: int - status of each chosen action */
int developerMenuSwitcher(struct client_data *data) {

    char *menuPointString = readMessage(data->newsockfd, 0);

    long menuPoint = strtol(menuPointString, NULL, 10);

    switch (menuPoint) {
        case 1:
            printRepositoryInfo(data, 1);
            return 1;
        case 2:
            markBugAsFixed(data);
            return 2;
        case 3:
            closeSocket(data->newsockfd);
            return -1;
        default:
            writeMessage(data->newsockfd, "Error choosing menu\n", data->pthread);
            return 0;
    }
}

/**There is a function that displays the menu depending on the position of the user
 * param: int position - 1 -> user is a tester, 0 -> user is a developer
 *        struct client_data - structure with information about each user
 * return: void*/
void printMenu(int position, struct client_data *data) {

    switch (position) {
        case 1:
            writeMessage(data->newsockfd, "1. List of fixed bugs\n", data->pthread);
            writeMessage(data->newsockfd, "2. List of active bugs\n", data->pthread);
            writeMessage(data->newsockfd, "3. Report a new bug\n", data->pthread);
            writeMessage(data->newsockfd, "4. Confirm or reject a bugfix\n", data->pthread);
            writeMessage(data->newsockfd, "5. Exit\n\n", data->pthread);
            break;
        case 0:
            writeMessage(data->newsockfd, "1. List of bugs found in your project\n", data->pthread);
            writeMessage(data->newsockfd, "2. Mark bug as fixed\n", data->pthread);
            writeMessage(data->newsockfd, "3. Exit\n\n", data->pthread);
            break;
        default:
            break;
    }
}