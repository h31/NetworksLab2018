#include "serverActions.h"

/**Here is function for receiving a login from client
 * We get login, compare it with the list of registered users
 * and interpret the user - he is a tester or developer
 * If user's login is ok, function add user to client structure
 * param: struct client_data *data - structure with information about each user
 * return: int position - 1 -> user is a tester, 0 -> user is a developer*/
int receiveLogin(struct client_data *data) {
    writeMessage(data->newsockfd, "Enter your login   ", data->pthread);

    char *buffer = readMessage(data->newsockfd, data->pthread);

    FILE *mFile = openFile("../res/logins");
    char *login;
    int position;
    bool checkLoop = false;

    while ((login = readOneLogin(mFile, &position)) != NULL) {

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, login) == 0) {

            writeMessage(data->newsockfd, "Your login is okey\n", data->pthread);
            checkLoop = true;

            if (position == 1) {
                writeMessage(data->newsockfd, "You are a tester\n", data->pthread);
            } else if (position == 0) {
                writeMessage(data->newsockfd, "You are a developer\n", data->pthread);
            }
            break;
        }
    }

    if (checkLoop) {
        printf("Here is the login: %s\n", buffer);
        data->login = malloc(MAX_BUFFER/2* sizeof(char));
        strcpy(data->login, buffer);
    } else {
        printf("Invalid login\n");
        return -1;
    }

    closeFile(mFile);
    free(buffer);

    return position;
}

/**Here is function for printing list of active client on the server
 * Function works with global client_array - array of client_data structures
 * param: void
 * return: void*/
void printClients() {

    int numberOfClients = 0;

    for (int i = 0; i < N; i++) {
        if (client_array[i] == NULL || client_array[i]->newsockfd == -1) {
            numberOfClients++;
        }
    }

    if (numberOfClients == N) {
        printf("There are no clients\n");
    } else {
        printf("Now working on the server:\n");
        printf("%-7s %-10s %-10s %-5s\n", "ID", "login", "IP", "port");
        for (int i = 0; i < N; i++) {
            if (client_array[i] != NULL && client_array[i]->newsockfd != -1) {
                printf("%-7d %-10s %-10ul %-5d\n",
                       client_array[i]->serial_num, client_array[i]->login,
                       client_array[i]->cli_addr.sin_addr.s_addr, client_array[i]->cli_addr.sin_port);
            }
        }
    }
}