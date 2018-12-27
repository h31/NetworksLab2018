#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <cygwin/socket.h>
#include <sys/socket.h>
#include <strings.h>
#include <stdlib.h>
#include <cygwin/in.h>
#include <memory.h>
#include <stdbool.h>


#define OPEN 0
#define CLOSED 1
#define MAX_LENGTH 256
#define MAX_LENGTH_NAME 50

void *connection_f();

void *connect_handler(void *args);

void sendErrorCode(int newsockfd, int type);

void sendSuccessCode(int newsockfd);

void actionLogin(char *buffer, int newsockfd);

void actionMenu(char *buffer, int newsockfd);

char *actionGetUser(char *buffer, int newsockfd);

void sentToUser(char *user, char *buffer, int newsockfd);

void actionSendBroadcastMsg(char *buffer, int newsockfd);

void actionSendMsg(char *user, char *buffer, int newsockfd);

void reading(int newsockfd, char *buffer);

void writing(int newsockfd, char *buffer);

void broadCastFromServer();

void msgFromServer();


int sockfd;
int isClose = OPEN;

void clear_all() {
    isClose = CLOSED;
    FILE *toDel = fopen("online.txt", "w+");
    fclose(toDel);
    printf("Closing server\r\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void deleteConnection(int newsockfd) {
    printf("Closing socket = %d \r\n", newsockfd);
    shutdown(newsockfd, SHUT_RDWR);
    char tokenName[MAX_LENGTH_NAME] = " ";
    bzero(tokenName, MAX_LENGTH_NAME);
    char tokenFile[MAX_LENGTH];
    bzero(tokenFile, MAX_LENGTH);
    char name[MAX_LENGTH_NAME] = " ";
    bzero(name, MAX_LENGTH_NAME);
    sprintf(tokenName, "%d", newsockfd);
    sprintf(tokenFile, "%d.txt", newsockfd);
    FILE *token = fopen(tokenFile, "r");
    fgets(name, MAX_LENGTH, token);
    remove(tokenFile);
    FILE *currOn = fopen("online.txt", "r");
    char temp[MAX_LENGTH];
    char fromFile[MAX_LENGTH];
    bzero(fromFile, MAX_LENGTH);
    bzero(temp, MAX_LENGTH);
    while (fgets(fromFile, MAX_LENGTH, currOn)) {
        strcat(temp, fromFile);
    };
    int ind = (int) (strstr(temp, tokenName) - temp);
    int ind_r = (int) strlen(name) + 3;
    fseek(currOn, ind, SEEK_SET);
    fseek(currOn, ind_r, SEEK_CUR);
    char secPart[MAX_LENGTH];
    bzero(secPart, MAX_LENGTH);
    while (fgets(fromFile, MAX_LENGTH, currOn)) {
        strcat(secPart, fromFile);
    };
    //fgets(secPart, MAX_LENGTH, currOn);
    printf("SECOND PART %s\n", secPart);
    printf("INDEX OF USER %d\n", ind);
    char final[MAX_LENGTH];
    bzero(final, MAX_LENGTH);
    strncpy(final, temp, ind);
    strcat(final, secPart);
    fclose(currOn);
    currOn = fopen("online.txt", "w+");
    fprintf(currOn, "%s", final);
    fclose(currOn);
    printf("FINAL STR %s\n", final);
    close(newsockfd);
    pthread_exit(0);
}

int sockfd;
enum ACTION {
    signin,
    mainmenu,
    checkUsers,
    sendMsg,
    sendBroadcastMsg
} act;

int main(int argc, char *argv[]) {
    //CTRL+C
    signal(SIGINT, clear_all);

    uint16_t portno;
    struct sockaddr_in serv_addr;
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
    /* Making new thread for messaging with client */
    pthread_t thread;    //thread
    int result;        //result of thread creating
    result = pthread_create(&thread, NULL, connection_f, NULL); //create new thread
    if (result != 0) {
        perror("Error while creating thread");
    }
    char cmd[MAX_LENGTH];
    bzero(cmd, MAX_LENGTH);

    while (!isClose) {
        fgets(cmd, 256, stdin);
        if (strncmp(cmd, "q", 1) == 0) {
            clear_all();
            return 0;
        } else if (strncmp(cmd, "broad", 5) == 0) {
            broadCastFromServer();
        } else if (strncmp(cmd, "msg", 3) == 0) {
            msgFromServer();
        }
    }
    clear_all();
    return 0;
}

void *connection_f() {
    /* Accept actual connection from the client */
    struct sockaddr_in cli_addr;
    unsigned int clilen;
    int newsockfd;
    int n;

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    pthread_t threadCreation;

    while (!isClose) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd <= 0) {
            perror("ERROR on accept");
            continue;
        }
        n = pthread_create(&threadCreation, NULL, connect_handler, (void *) newsockfd);
        if (n != 0) {
            printf("Error on creating client thread");
        }
    }

    pthread_exit(0);
}

void *connect_handler(void *args) {
    int newsockfd = (int *) args;
    char buffer[MAX_LENGTH];
    char *getUser;
    ssize_t n;
    bool isLog = false;
    act = signin;
    while (!isClose) {
        switch (act) {
            case signin: {
                actionLogin(buffer, newsockfd);
                break;
            }
            case mainmenu: {
                actionMenu(buffer, newsockfd);
                break;
            }
            case checkUsers: {
                getUser = actionGetUser(buffer, newsockfd);
                printf("YA VIBRAL %s \n", getUser);
                break;
            }
            case sendMsg: {
                actionSendMsg(getUser, buffer, newsockfd);
                break;
            }
            case sendBroadcastMsg: {
                actionSendBroadcastMsg(buffer, newsockfd);
                break;
            }
        }
    }
    deleteConnection(newsockfd);
}

void broadCastFromServer() {
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Введите сообщение для всех пользователей\n");
    strcat(buffer, "<0> Отмена\n");
    char getUser[MAX_LENGTH];
    FILE *file = fopen("online.txt",
                       "r");
    printf("%s\n", buffer);
    int key = 0;
    bool isChosed = false;
    while (!isChosed) {
        bzero(buffer, MAX_LENGTH);
        fgets(buffer, 256, stdin);
        if (strncmp(buffer, "0", 1) == 0) {
            break;
        } else {
            while (fgets(getUser, MAX_LENGTH, file)) {
                printf("%d-ый ПОШЕЛ!\n", key);
                key++;
                sentToUser(getUser, buffer, -1);
            }
            fclose(file);
            isChosed = true;
            act = sendBroadcastMsg;
        }

    }
    bzero(buffer, MAX_LENGTH);
}

void msgFromServer() {
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);
    char *getStr = calloc(MAX_LENGTH, MAX_LENGTH);
    FILE *file = fopen(
            "online.txt",
            "r");
    char getUser[MAX_LENGTH];
    int cnt = 1;
    char temp[10];
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Пользователи онлайн: \n");
    while (fgets(getUser, MAX_LENGTH, file)) {
        sprintf(temp, "<%d> ", cnt);
        strcat(buffer, temp);
        strcat(buffer, getUser);
        cnt++;
    };
    strcat(buffer, "<0> Вернуться в меню\n");
    fclose(file);
    file = fopen("online.txt",
                 "r");
    printf("%s \n", buffer);
    bool isCorrect = false;
    while (!isCorrect) {
        bzero(buffer, MAX_LENGTH);
        fgets(buffer, 256, stdin);
        bzero(getStr, MAX_LENGTH);
        int count = atoi(&buffer[0]);
        if (count >= cnt) {
            printf("Нет такого пользователя");
        } else if (count == 0) {
            isCorrect = true;
        } else {
            for (int i = 0; i < count; i++) {
                fgets(getStr, MAX_LENGTH, file);
                if (feof(file)) break;
            }
            fclose(file);
            isCorrect = true;
        }
    }
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Введите сообщение\n");
    strcat(buffer, "<0> Вернуться в меню\n");
    printf("%s\n", buffer);
    bool isChosed = false;
    while (!isChosed) {
        bzero(buffer, MAX_LENGTH);
        fgets(buffer, 256, stdin);
        if (strncmp(buffer, "0", 1) == 0) {
            isChosed = true;
        } else {
            sentToUser(getStr, buffer, -1);
            isChosed = true;
        }

    }
    bzero(buffer, MAX_LENGTH);
}

void sendErrorCode(int newsockfd, int type) {
    char buffer[MAX_LENGTH];
    bzero(buffer, MAX_LENGTH);
    ssize_t n = write(newsockfd, "STATUS -1", MAX_LENGTH); // recv on Windows
    if (n <= 0) {
        perror("ERROR reading from socket");
        deleteConnection(newsockfd);
    }
    switch (type) {
        case 400: {
            strcat(buffer, "Неправильный запрос");
            break;
        }
        case 401: {
            strcat(buffer, "Неверное имя пользователя, попробуйте еще раз");
            break;
        }
    }
    n = write(newsockfd, buffer, MAX_LENGTH);
    if (n <= 0) {
        perror("ERROR reading from socket");
        deleteConnection(newsockfd);
    }
};

void sendSuccessCode(int newsockfd) {
    ssize_t n = write(newsockfd, "STATUS 0", MAX_LENGTH); // recv on Windows
    if (n <= 0) {
        perror("ERROR reading from socket");
        deleteConnection(newsockfd);
    }
};

void reading(int newsockfd, char *buffer) {
    ssize_t n = read(newsockfd, buffer, MAX_LENGTH); // recv on Windows
    if (n <= 0) {
        perror("ERROR reading from socket");
        deleteConnection(newsockfd);
    }
    printf("От %d сокета принято сообщение %s", newsockfd, buffer);
}

void writing(int newsockfd, char *buffer) {
    sendSuccessCode(newsockfd);
    ssize_t n = write(newsockfd, buffer, MAX_LENGTH); // send on Windows
    if (n <= 0) {
        perror("ERROR writing to socket");
        deleteConnection(newsockfd);
    }
    bzero(buffer, MAX_LENGTH);
}

void actionLogin(char *buffer, int newsockfd) {
    FILE *token;
    bool isLog = false;
    while (!isLog) {
        strcat(buffer, "Введите логин");
        writing(newsockfd, buffer);
        while (!isLog) {
            reading(newsockfd, buffer);
            FILE *file = fopen(
                    "USERS.txt",
                    "r");
            char getUser[MAX_LENGTH];
            while (fgets(getUser, MAX_LENGTH, file)) {
                if (strncmp(getUser, buffer, strlen(getUser) - 2) == 0) {
                    isLog = true;
                    FILE *currOn = fopen("online.txt", "a+");
                    char filename[5];
                    sprintf(filename, "%d", newsockfd);
                    strcat(filename, ".txt");
                    token = fopen(filename, "w+");
                    int ind = (int) (strchr(getUser, '\n') - getUser + 1);
                    char name[MAX_LENGTH_NAME];
                    strncpy(name, getUser, ind - 1);
                    fprintf(token, "%s", name);
                    fprintf(currOn, "%d %s\n", newsockfd, name);
                    fclose(currOn);
                    fclose(token);
                }
            };
            if (!isLog) {
                bzero(buffer, MAX_LENGTH);
                sendErrorCode(newsockfd, 401);
            }
            fclose(file);
        }
        act = mainmenu;
        bzero(buffer, MAX_LENGTH);

    }
}

void actionMenu(char *buffer, int newsockfd) {
    FILE *file = fopen(
            "menu.txt",
            "r");
    char getUser[MAX_LENGTH];
    strcat(buffer, "Доступные действия: \n");
    while (!feof(file)) {
        fgets(getUser, MAX_LENGTH, file);
        strcat(buffer, getUser);
    };
    writing(newsockfd, buffer);
    bool isCorrect = false;
    while (!isCorrect) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        if (strncmp(buffer, "1", 1) == 0) {
            act = checkUsers;
            isCorrect = true;
        } else if (strncmp(buffer, "2", 1) == 0) {
            act = sendBroadcastMsg;
            isCorrect = true;
        } else {
            sendErrorCode(newsockfd, 400);
        }
    }
    fclose(file);
    bzero(buffer, MAX_LENGTH);
}

char *actionGetUser(char *buffer, int newsockfd) {
    FILE *file = fopen(
            "online.txt",
            "r");
    char getUser[MAX_LENGTH];
    int cnt = 1;
    char temp[10];
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Пользователи онлайн: \n");
    while (fgets(getUser, MAX_LENGTH, file)) {
        sprintf(temp, "<%d> ", cnt);
        strcat(buffer, temp);
        strcat(buffer, getUser);
        cnt++;
    };
    strcat(buffer, "<0> Вернуться в меню\n");
    fclose(file);
    file = fopen("online.txt",
                 "r");
    writing(newsockfd, buffer);
    bool isCorrect = false;
    while (!isCorrect) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        char *getStr = calloc(MAX_LENGTH, MAX_LENGTH);
        bzero(getStr, MAX_LENGTH);
        int count = atoi(&buffer[0]);
        if (count >= cnt) {
            sendErrorCode(newsockfd, 400);
        } else if (count == 0) {
            isCorrect = true;
            act = mainmenu;
        } else {
            for (int i = 0; i < count; i++) {
                fgets(getStr, MAX_LENGTH, file);
                if (feof(file)) break;
            }
            fclose(file);
            isCorrect = true;
            act = sendMsg;
            return getStr;
        }
    }
    bzero(buffer, MAX_LENGTH);
}

void actionSendMsg(char *user, char *buffer, int newsockfd) {
    strcat(buffer, "Введите сообщение\n");
    strcat(buffer, "<0> Вернуться в меню\n");
    writing(newsockfd, buffer);
    bool isChosed = false;
    while (!isChosed) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        if (strncmp(buffer, "0", 1) == 0) {
            isChosed = true;
            act = mainmenu;
        } else {
            sentToUser(user, buffer, newsockfd);
            isChosed = true;
            //act = mainmenu;
        }

    }
    bzero(buffer, MAX_LENGTH);
}


void sentToUser(char *user, char *buffer, int newsockfd) {
    ssize_t ind = (strchr(user, ' ') - user);
    char *sock = calloc(MAX_LENGTH_NAME, MAX_LENGTH_NAME);
    int sockNum = atoi(strncpy(sock, user, ind));
    char tmpbuffer[MAX_LENGTH];
    char src[MAX_LENGTH_NAME];
    char srcName[MAX_LENGTH_NAME] = " ";
    if (newsockfd != -1) {
        sprintf(src, "%d", newsockfd);
        strcat(src, ".txt");
        FILE *srcFile = fopen(src, "r");
        fgets(srcName, MAX_LENGTH_NAME, srcFile);
        int kek = strlen(srcName) - 1;
        char temp[kek];
        strncpy(temp, srcName, kek);
        sprintf(tmpbuffer, "Вам пришло сообщение от %s: \n", temp);
        strcat(tmpbuffer, buffer);
        writing(sockNum, tmpbuffer);
    } else {
        sprintf(srcName, "SERVER");
        sprintf(tmpbuffer, "Вам пришло сообщение от %s: \n", srcName);
        strcat(tmpbuffer, buffer);
        writing(sockNum, tmpbuffer);
    }
}

void actionSendBroadcastMsg(char *buffer, int newsockfd) {
    strcat(buffer, "Введите сообщение для всех пользователей\n");
    strcat(buffer, "<0> Вернуться в меню\n");
    char getUser[MAX_LENGTH];
    FILE *file = fopen("online.txt",
                       "r");
    writing(newsockfd, buffer);
    int key = 0;
    bool isChosed = false;
    while (!isChosed) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        if (strncmp(buffer, "0", 1) == 0) {
            isChosed = true;
            act = mainmenu;
        } else {
            while (fgets(getUser, MAX_LENGTH, file)) {
                printf("%d-ый ПОШЕЛ!\n", key);
                key++;
                sentToUser(getUser, buffer, newsockfd);
            }
            fclose(file);
            isChosed = true;
            act = sendBroadcastMsg;
        }

    }
    bzero(buffer, MAX_LENGTH);
}