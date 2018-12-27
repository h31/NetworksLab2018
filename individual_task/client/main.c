#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <cygwin/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <asm/byteorder.h>
#include <cygwin/in.h>
#include <memory.h>
#include <netdb.h>
#include <stdbool.h>

int sockfd;

void closeApp() {
    printf("Closing socket\r\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(0);
}

void *getMsg(void *args);

void *putMsg(void *args);

int main(int argc, char *argv[]) {
    signal(SIGINT, closeApp);
    ssize_t n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = (uint16_t) atoi(argv[2]);
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);
    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    /* Now ask for a message from the user, this message
      * will be read by server
   */
    pthread_t readThread;    //thread
    int readResult;        //result of thread creating
    readResult = pthread_create(&readThread, NULL, getMsg, (void *) sockfd); //create new thread
    if (readResult != 0) {
        perror("Error while creating thread");
    }
    pthread_t writeThread;    //thread
    int result;        //result of thread creating
    result = pthread_create(&writeThread, NULL, putMsg, (void *) sockfd); //create new thread
    if (result != 0) {
        perror("Error while creating thread");
    }

    bool isClosed = false;
    while (!isClosed) {

    }
    return 0;
}

void *getMsg(void *args) {
    int newsockfd = (int *) args;
    bool isOk = false;
    char buffer[256];
    while (!isOk) {
        //printf("Я В ПОТОКЕ ЧТЕНИЯ\n");
        bzero(buffer, 256);
        ssize_t n = read(newsockfd, buffer, 256);
        if (n <= 0) {
            perror("ERROR reading from socket");
            isOk = true;
            closeApp();
        }
        printf("%s\n", buffer);
        char *temp = calloc(50, 50);
        strncpy(temp, buffer, 16);
        if (strncmp(temp, "You've got a msg", 16) == 0) {
            printf("Пришло сообщение %s \n", buffer);
        }
    }
    pthread_exit(0);
}

void *putMsg(void *args) {
    int newsockfd = (int *) args;
    bool isOk = false;
    char buffer[256];
    while (!isOk) {
        bzero(buffer, 256);
        fgets(buffer, 256, stdin);
        if (strstr(buffer, "\\q") != NULL) {
            closeApp();
        }
        //printf("Я В ПОТОКЕ ЗАПИСИ\n");
        ssize_t n = write(newsockfd, buffer, 256);
        if (n <= 0) {
            isOk = true;
            perror("ERROR reading from socket");
            closeApp();
        }
    }
    pthread_exit(0);
}