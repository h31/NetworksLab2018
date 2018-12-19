#include "includes.h"

int bugs_number = 0;
struct repository_data *rep_data;
long bugId;

bool run = true;
struct client_data *client_array[N] = {};
int client_serial_num = 0;
int sockfd;

int main(int argc, char *argv[]) {
    int newsockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* Create new socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    /* Initialize a socket structure */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)) < 0) {
        perror("ERROR on setsockopt");
        closeSocket(sockfd);

        exit(1);
    }

    /* Bind socket to server address */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        closeSocket(sockfd);

        exit(1);
    }

    /* Listen to the client */
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    pthread_t handlerKeyThread;
    pthread_create(&handlerKeyThread, NULL, waitKeyThread, NULL);

    /**Read data about repositories into struct array
     * and fill rep_data array*/
    FILE *file = openFile("../res/repositories");

    rep_data = malloc(MAX_REPO * sizeof(struct repository_data));

    for (int i = 0; i < MAX_REPO; i++) {
        rep_data[i].dev_login = malloc(100 * sizeof(char));
        rep_data[i].test_login = malloc(100 * sizeof(char));
        rep_data[i].project = malloc(100 * sizeof(char));
        rep_data[i].bugId = 0;
        rep_data[i].bugStatus = malloc(100 * sizeof(char));
        rep_data[i].bugDescription = malloc(100 * sizeof(char));
    }

    bugs_number = readRepositoryInfo(file);

    closeFile(file);

    /* Wait for connection*/
    while (run) {

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0) {
            perror("ERROR on accept");
            closeSocket(sockfd);
            continue;
        }

        /**Check for server fullness
         * If a new client has connected, but too many clients are already connected
         * new client connects instead of first, another one - instead of second
         * and so on */
        if (client_serial_num >= N) {
            printf("the end of client size\n");
            if (client_array[client_serial_num % N]->newsockfd != -1) {
                closeSocket(client_array[client_serial_num % N]->newsockfd);
            }
            free(client_array[client_serial_num % N]);
            client_array[client_serial_num % N] = NULL;
        }

        /* Initialize a client_data structure */
        struct client_data *data = malloc(sizeof(struct client_data));
        data->newsockfd = newsockfd;
        data->cli_addr = cli_addr;
        data->serial_num = client_serial_num;

        /* Fill array of client_data structures */
        client_array[client_serial_num % N] = data;
        client_serial_num++;

        /* Create thread for communication with new client */
        pthread_t communicThread;
        int result;
        result = pthread_create(&communicThread, NULL, communicateThread, (void *) data);
        if (result != 0) {
            perror("error creating thread");
            closeSocket(newsockfd);
        }
        data->pthread = communicThread;
    }

}