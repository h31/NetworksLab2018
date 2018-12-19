#include "serverThreads.h"

/**Here is function for thread that is created for communication with a clients
 * param: void *thread_data - struct client_data - structure with information about each user
 * return: void* */
void *communicateThread(void *thread_data) {

    struct client_data *data = (struct client_data *) thread_data;

    int position;
    while ((position = receiveLogin(data)) < 0) {
        writeMessage(data->newsockfd, "Please, try again\n", data->pthread);
    }

    while (optionsMenuListener(position, data) >= 0);

    closeSocket(data->newsockfd);
    data->newsockfd = -1;

    return 0;
}

/**Here is function to listen for keystroke on the server
 * If key q - end of session
 * key l - print list of active clients
 * key d <id> - disconnect client with <id>
 *
 * In the end of session server overwrites file with repository information
 * param: void
 * return: void* */
void *waitKeyThread() {

    char key;

    L:
    while ((key = (char) getchar()) != 'q') {
        if (key == 'l') {
            printClients();
        }
        if (key == 'd') {
            char *id = NULL;
            ssize_t size;
            getline(&id, &size, stdin);
            id[strcspn(id, "\n")] = 0;
            for (int i = 0; i < N; i++) {
                if (client_array[i] != NULL && client_array[i]->newsockfd != -1 &&
                    client_array[i]->serial_num == atoi(id)) {
                    closeSocket(client_array[i]->newsockfd);
                    free(client_array[i]);
                    client_array[i] = NULL;
                    goto L;
                }
            }
            printf("No such id\n");
        }
    }
    run = false;
    closeSocket(sockfd);

    for (int i = 0; i < N; i++) {
        if (client_array[i] != NULL && client_array[i]->newsockfd != -1) {
            closeSocket(client_array[i]->newsockfd);
        }
    }

    FILE *file = openFile("../res/repositories");
    writeToRepositories(file);
    closeFile(file);
    return 0;
}