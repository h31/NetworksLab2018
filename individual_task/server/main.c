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

char *actionThemes(char *buffer, int newsockfd);

void sendErrorCode(int newsockfd, int type);

void sendSuccessCode(int newsockfd);

void actionLogin(char *buffer, int newsockfd);

void actionMenu(char *buffer, int newsockfd);

void actionMessages(char *theme, char *buffer, int newsockfd);

void actionNewMessage(char *buffer, int newsockfd);

void checkOnline(char *buffer, int newsockfd);

void reading(int newsockfd, char *buffer);

void writing(int newsockfd, char *buffer);

void putNewMsg(char *newMessage);


int sockfd;
int all_clients = 0;
int isClose = OPEN;
int countOfNewMsg = 3;

void clear_all() {
    isClose = CLOSED;
    FILE *toDel = fopen("currentOn.txt", "w+");
    fclose(toDel);
    printf("Closing server\r\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
}

void deleteConnection(int newsockfd) {
    printf("Closing socket = %d \r\n", newsockfd);
    shutdown(newsockfd, SHUT_RDWR);
    char tokenName[MAX_LENGTH_NAME];
    char name[MAX_LENGTH_NAME];
    bzero(name, MAX_LENGTH_NAME);
    FILE *token;
    sprintf(tokenName, "%d", newsockfd);
    strcat(tokenName, ".txt");
    token = fopen(tokenName, "r");
    fgets(name, MAX_LENGTH_NAME, token);
    remove(tokenName);
    FILE *currOn = fopen("currentOn.txt", "r");
    char temp[MAX_LENGTH];
    bzero(temp, MAX_LENGTH);
    fgets(temp, MAX_LENGTH, currOn);
    int ind = (int) (strstr(temp, name) - temp);
    fseek(currOn, ind, SEEK_SET);
    fseek(currOn, strlen(name), SEEK_CUR);
    char secPart[MAX_LENGTH];
    bzero(secPart, MAX_LENGTH);
    fgets(secPart, MAX_LENGTH, currOn);
    printf("SECOND PART %s\n", secPart);
    printf("INDEX OF USER %d\n", ind);
    char final[MAX_LENGTH];
    bzero(final, MAX_LENGTH);
    strncpy(final, temp, ind);
    strcat(final, secPart);
    fclose(currOn);
    currOn = fopen("currentOn.txt", "w+");
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
    checkThemes,
    checknews,
    checkMessages,
    checkonline
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
    portno = 8080;
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
    char cmd;

    while (!isClose) {
        cmd = getchar();
        if (cmd == 'q') {
            break;
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
    char *getTheme;
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
            case checkThemes: {
                getTheme = actionThemes(buffer, newsockfd);
                break;
            }
            case checkMessages: {
                actionMessages(getTheme, buffer, newsockfd);
                break;
            }
            case checknews: {
                actionNewMessage(buffer, newsockfd);
                break;
            }
            case checkonline: {
                checkOnline(buffer, newsockfd);
                break;
            }
        }
    }
    deleteConnection(newsockfd);
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
            strcat(buffer, "Неверное имя пользователя или пароль, попробуйте еще раз");
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
        strcat(buffer, "Введите логин и пароль в формате (логин_пароль)");
        writing(newsockfd, buffer);
        while (!isLog) {
            reading(newsockfd, buffer);
            FILE *file = fopen(
                    "C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\USERS.txt",
                    "r");
            char getUser[MAX_LENGTH];
            while (!feof(file)) {
                fgets(getUser, MAX_LENGTH, file);
                if (strncmp(getUser, buffer, strlen(getUser) - 2) == 0) {
                    isLog = true;
                    FILE *currOn = fopen("currentOn.txt", "a+");
                    char filename[5];
                    sprintf(filename, "%d", newsockfd);
                    strcat(filename, ".txt");
                    token = fopen(filename, "w+");
                    int ind = (int) (strchr(getUser, '_') - getUser + 1);
                    char name[MAX_LENGTH_NAME];
                    strncpy(name, getUser, ind - 1);
                    fprintf(token, "%s", name);
                    fprintf(currOn, "%s ", name);
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
        /* Write a response to the client */
        if (isLog) {
            act = mainmenu;
        }
        bzero(buffer, MAX_LENGTH);

    }
}

void actionMenu(char *buffer, int newsockfd) {
    FILE *file = fopen(
            "C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\menu.txt",
            "r");
    char getUser[MAX_LENGTH];
    strcat(buffer, "Навигация форума: \n");
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
            act = checkThemes;
            isCorrect = true;
        } else if (strncmp(buffer, "2", 1) == 0) {
            act = checknews;
            isCorrect = true;
        } else if (strncmp(buffer, "3", 1) == 0) {
            act = checkonline;
            isCorrect = true;
        } else {
            sendErrorCode(newsockfd, 400);
        }
    }
    fclose(file);
    bzero(buffer, MAX_LENGTH);
}

char *actionThemes(char *buffer, int newsockfd) {
    FILE *file = fopen(
            "C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\themes.txt",
            "r");
    char getUser[MAX_LENGTH];
    int cnt = 1;
    char temp[10];
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Существующие темы: \n");
    while (fgets(getUser, MAX_LENGTH, file)) {
        sprintf(temp, "<%d> ", cnt);
        strcat(buffer, temp);
        strcat(buffer, getUser);
        cnt++;
    };
    strcat(buffer, "<0> Вернуться в меню\n");
    fclose(file);
    file = fopen("C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\themes.txt",
                 "r");
    writing(newsockfd, buffer);
    bool isCorrect = false;
    while (!isCorrect) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        char *getStr = calloc(MAX_LENGTH, MAX_LENGTH);
        bzero(getStr, MAX_LENGTH);
        char *forN = calloc(MAX_LENGTH, MAX_LENGTH);
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
            act = checkMessages;
            return getStr;
        }
    }
    bzero(buffer, MAX_LENGTH);
}

void actionMessages(char *theme, char *buffer, int newsockfd) {
    char openFilePath[MAX_LENGTH];
    char username[MAX_LENGTH_NAME];
    char fileUsername[MAX_LENGTH_NAME];
    bzero(username, MAX_LENGTH_NAME);
    bzero(fileUsername, MAX_LENGTH_NAME);
    sprintf(fileUsername, "%d", newsockfd);
    strcat(fileUsername, ".txt");
    FILE *token = fopen(fileUsername, "r");
    fgets(username, MAX_LENGTH_NAME, token);
    sprintf(openFilePath, "themes\\");
    strncat(openFilePath, theme, strlen(theme) - 2);
    strcat(openFilePath, ".txt");
    printf("FILE PATH %s\n", openFilePath);
    FILE *file;
    if ((file = fopen(openFilePath, "r")) == NULL) {
        printf("Cannot open FILE");
    }
    char getMsg[MAX_LENGTH];
    bzero(getMsg, MAX_LENGTH);
    int cnt = 1;
    char temp[10];
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Посты: \n");
    while (fgets(getMsg, MAX_LENGTH, file)) {
        sprintf(temp, "%d. ", cnt);
        strcat(buffer, temp);
        strcat(buffer, getMsg);
        cnt++;
    };
    strcat(buffer, "<1> Написать пост\n");
    strcat(buffer, "<0> Вернуться в меню\n");
    fclose(file);
    file = fopen(openFilePath, "a+");
    writing(newsockfd, buffer);
    bool isChosed = false;
    while (!isChosed) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        int count = atoi(&buffer[0]);
        if (count == 0) {
            isChosed = true;
            act = mainmenu;
        } else if (count == 1) {
            bzero(buffer, MAX_LENGTH);
            strcat(buffer, "Введите текст поста \n<0> Для отмены");
            writing(newsockfd, buffer);
            bzero(buffer, MAX_LENGTH);
            reading(newsockfd, buffer);
            if (strncmp(buffer, "0", 1) == 0) {
                act = checkMessages;
                isChosed = true;
                fclose(file);
            } else {
                fprintf(file, "%s запостил: %s", username, buffer);
                char toNew[MAX_LENGTH];
                char currTheme[MAX_LENGTH];
                bzero(currTheme, MAX_LENGTH);
                strncat(currTheme, theme, strlen(theme) - 2);
                bzero(toNew, MAX_LENGTH);
                sprintf(toNew, "В %s %s запостил: %s", currTheme, username, buffer);
                putNewMsg(toNew);
                act = checkMessages;
                isChosed = true;
                fclose(file);
            }
        } else {
            sendErrorCode(newsockfd, 400);
        }
    }
}

void actionNewMessage(char *buffer, int newsockfd) {
    FILE *file = fopen("C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\lastmsg.txt", "r");
    char getNMsg[MAX_LENGTH];
    int cnt = 1;
    char temp[10];
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Последние посты: \n");
    while (fgets(getNMsg, MAX_LENGTH, file)) {
        sprintf(temp, "%d. ", cnt);
        strcat(buffer, temp);
        strcat(buffer, getNMsg);
        cnt++;
    };
    strcat(buffer, "<0> Вернуться в меню\n");
    fclose(file);
    writing(newsockfd, buffer);
    bzero(buffer, MAX_LENGTH);
    bool isCorrect = false;
    while (!isCorrect) {
        reading(newsockfd, buffer);
        int count = atoi(&buffer[0]);
        if (count == 0) {
            act = mainmenu;
            isCorrect = true;
        } else {
            sendErrorCode(newsockfd, 400);
        }
        bzero(buffer, MAX_LENGTH);
    }
}

void putNewMsg(char *newMessage) {
    FILE *fileWithNewMsg = fopen(
            "C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\lastmsg.txt", "r");
    char tempBuffer[MAX_LENGTH];
    char getStr[MAX_LENGTH];
    char forN[2];
    bzero(tempBuffer, MAX_LENGTH);
    bzero(getStr, MAX_LENGTH);
    bzero(forN, MAX_LENGTH);
    for (int i = 0; i < countOfNewMsg - 1; i++) {
        fgets(getStr, sizeof(getStr), fileWithNewMsg);
        strcat(tempBuffer, getStr);
    }
    strcat(newMessage, tempBuffer);
    fclose(fileWithNewMsg);
    fileWithNewMsg = fopen(
            "C:\\Users\\User\\CLionProjects\\individual-task\\server\\cmake-build-debug\\lastmsg.txt", "w+");
    fprintf(fileWithNewMsg, "%s", newMessage);
    fclose(fileWithNewMsg);
}

void checkOnline(char *buffer, int newsockfd) {
    FILE *file = fopen(
            "currentOn.txt",
            "r");
    char getUsers[MAX_LENGTH];
    bzero(getUsers, MAX_LENGTH);
    bzero(buffer, MAX_LENGTH);
    strcat(buffer, "Пользователи онлайн: \n");
    while (fgets(getUsers, MAX_LENGTH, file)) {
        strcat(buffer, getUsers);
    };
    strcat(buffer, "\n<0> Вернуться в меню\n");
    fclose(file);
    writing(newsockfd, buffer);
    bool isCorrect = false;
    while (!isCorrect) {
        bzero(buffer, MAX_LENGTH);
        reading(newsockfd, buffer);
        if (strncmp(buffer, "0", 1) == 0) {
            act = mainmenu;
            isCorrect = true;
        } else {
            sendErrorCode(newsockfd, 400);
        }
    }
}