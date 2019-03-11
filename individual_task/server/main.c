#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <locale.h>
#include <signal.h>
#include <errno.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <ctype.h>

#define PORT 5001  // port we're listening on
#define MAX_GAMERS 36
#define BUF_SIZE 24

char shutdown_serv[] = "SHUTDOWN";
char deactivate[] = "DEACT";
int clients_count = -1;
bool croupier_online = false;
bool game_process = false;
int result = -1;
int listener;// listening socket descriptor

char *password = "1366";

struct client {
    int bet;
    int sum;
    int socket_desc;
    bool isCroupier;
    bool isActive;
} *pClients;

enum step            /* Defines an enumeration type    */
{
    WELCOME,
    WAIT,
    RESULT
};

void ShutdownRoullete() {
    for (int i = 0; i <= clients_count; i++) {
        //shutdown(pClients[i].socket_desc, SHUT_RDWR);
        close(pClients[i].socket_desc);
    }
}

char CompareCharArrayIgnoreCase(const char *x, const char *y){
    while ((*x != '\0') && (*y != '\0')) { // testing y for symmetry
        if (tolower(*x) < tolower(*y)){
            return -1;
        }
        else if (tolower(*x) > tolower(*y)){
            return 1;
        }
        else{
            x++;
            y++;
        }
    }
    // added conditions to handle shorter/longer strings
    if  (*x == '\0')
    {
        if (*y == '\0')
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    return 1;
}




int SendToClient(int socket, char* message) {
    if (send(socket, message, BUF_SIZE, 0) < 0) {
        perror("Сообщение не отправлено клиенту. Сокет закрыт.");
        return -1;
    }
    return 1;
}

void KickClient(int id) {
    if (pClients[id].isCroupier) {
        croupier_online = false;
    }
    SendToClient(pClients[id].socket_desc, "KICKED");
    close(pClients[id].socket_desc);
    pClients[id].isActive = false;
    clients_count--;
}

int CroupierAuth(int socket) {
    char buf[BUF_SIZE];
    if (SendToClient(socket, "PASW") < 0)
        return -1;
    int count_attempt = 0;
    while (count_attempt < 3) {
        if (recv(socket, buf, BUF_SIZE, 0) < 0) {
            perror("Не удалось принять вызов.");
            close(socket);
            clients_count--;
            return -1;
        }
        else {
            count_attempt++;
            strtok(buf, "\n");
            if (CompareCharArrayIgnoreCase(password, buf) != 0) {
                if (count_attempt >= 3) {
                    printf("Крупье ввёл неверный пароль три раза.\n");
                    if (SendToClient(socket, "THIRDPASW") > 0) {
                        clients_count--;
                        close(socket);
                    }
                    return -1;
                }
                if (SendToClient(socket, "WRONGPASW") < 0)
                    return -1;
                memset(buf, 0, BUF_SIZE);
            }
            else {
                printf("Крупье зашёл.\n");
                SendToClient(socket, "LOGGED");
                memset(buf, 0, BUF_SIZE);
                break;
            }
        }
    }
    return 0;
}

int SentBetsList(int socket) {
    int count = 0, i = 0;
    while (count < clients_count) {
        if (!pClients[i].isCroupier && pClients[i].isActive) {
            char tmp[BUF_SIZE];
            snprintf(tmp, BUF_SIZE, "%s%d %d %d", "BET ", i, pClients[i].bet, pClients[i].sum);
            if (SendToClient(socket, tmp) < 0)
                return -1;
            count++;
        }
        i++;
    }
    if (SendToClient(socket, "STOPLIST") < 0)
        return -1;
    return 0;
}

void WaitResult() {
    for(;;) {
        if (game_process)
            break;
    }
    for(;;)
        if (!game_process)
            break;
}

int ResultRoulette(int socket) {
    game_process = true;
    result = rand() % 37;
    char tmp[BUF_SIZE];
    bzero(tmp, BUF_SIZE+1);
    snprintf(tmp, 10, "%s%d", "RESULT ", result);
    if (SendToClient(socket, tmp) < 0)
        return -1;
    game_process = false;
    return 0;
}

int AppointmentId() {
    for (int i = 0; i < MAX_GAMERS + 1; i++) {
        if (!pClients[i].isActive)
            return i;
    }
    return -1;
}


void *ClientHandler(void* current_index) {
    int socket = pClients[(int) current_index].socket_desc;
    pClients[(int)current_index].isActive = true;
    char buf[BUF_SIZE]; //Buffer
    char croupier[] = "CROUP";
    bool is_croupier = false;

    //recive login
    if (recv(socket, buf, BUF_SIZE, 0) < 0) {
        KickClient((int)current_index);
        pthread_exit(NULL);
    }
    if (clients_count >= MAX_GAMERS && CompareCharArrayIgnoreCase(buf, "GAMER") == 0) {
        if ((SendToClient(socket, "ONLYCROUP") < 0))
            printf("Сообщение ONLYCROUP отправляется на ID %d", (int)current_index);
        KickClient((int)current_index);
        pthread_exit(NULL);
    }
    //is name = croupier?
    if (strcmp(buf, croupier) == 0)
        is_croupier = true;
    memset(buf, 0, BUF_SIZE);
    if (is_croupier) {
        if (croupier_online) {
            printf("Клиент был удалён. Крупье уже существует.\n");
            if (SendToClient(socket, "CROUPEXIST") < 0 )
                printf("Сообщение CROUPEXIST отправляется на ID %d", (int)current_index);
            KickClient((int)current_index);
            pthread_exit(NULL);
        } else {
            if (CroupierAuth(socket) < 0) {
                KickClient((int)current_index);
                pthread_exit(NULL);
            }
            pClients[(int) current_index].isCroupier = true;
            croupier_online = true;
        }
    }
    else {
        printf("Клиент с ID: '%d' зашел.\n", (int) current_index);

        char tmp[BUF_SIZE];
        snprintf(tmp, BUF_SIZE, "%s%d", "GAMERID ", (int) current_index);
        if (SendToClient(socket, tmp) < 0) {
            KickClient((int)current_index);
            pthread_exit(NULL);
        }
    }
    //Working
    enum step currentStep = WELCOME;
    for(;;) {
        if (currentStep == WELCOME) {
            if (game_process) {
                if (SendToClient(socket, "GAMEPROCESS") < 0) {
                    KickClient((int)current_index);
                    pthread_exit(NULL);
                };
                continue;
            } else {
                if (SendToClient(socket, "WELCOME") < 0) {
                    KickClient((int)current_index);
                    pthread_exit(NULL);
                };
                memset(buf, 0, BUF_SIZE);
                if (recv(socket, buf, BUF_SIZE, 0) < 0) {
                    perror("Не удалось принять вызов.");
                    KickClient((int)current_index);
                    close(socket);
                    pthread_exit(NULL);
                }

                // Give list of bets all gamers
                if (strcmp(buf, "GIVELIST") == 0) {
                    if (SentBetsList(socket) < 0) {
                        KickClient((int) current_index);
                        pthread_exit(NULL);
                    }
                    continue;
                }

                if (strstr(buf, "MYBETIS") != NULL && !is_croupier) {
                    currentStep = WAIT;
                    size_t k = strlen(buf);
                    char *bet, *sum;
                    strtok(buf, " ");
                    bet = strtok(NULL, " ");
                    sum = strtok(NULL, " ");
                    pClients[(int) current_index].bet = atoi(bet);
                    pClients[(int) current_index].sum = atoi(sum);
                    continue;
                }
                if (strcmp(buf, "STARTGAME") == 0 && is_croupier) {
                    currentStep = RESULT;
                    game_process = true;
		    sleep(1);
                    continue;
                }
                if (strcmp(buf, "EXIT") == 0) {
                    KickClient((int)current_index);
                    pthread_exit(NULL);
                }
            }
        }
        if (currentStep == WAIT) {
            if (SendToClient(socket, "WAITGAME") < 0) {
                KickClient((int)current_index);
                pthread_exit(NULL);
            }
            WaitResult();
            if (!pClients[(int)current_index].isActive)
                pthread_exit(NULL);
            bzero(buf, BUF_SIZE+1);
            snprintf(buf, 10, "%s%d", "RESULT ", result);
            if (SendToClient(socket, buf) < 0) {
                KickClient((int)current_index);
                pthread_exit(NULL);
            }
            currentStep = WELCOME;
        }
        if (currentStep == RESULT && is_croupier) {
            if (ResultRoulette(socket) < 0) {
                KickClient((int)current_index);
                pthread_exit(NULL);
            }
            currentStep = WELCOME;
        }
        memset(buf, 0, BUF_SIZE);
    }
}


void *ServerHandler(void* empty) {
    char input[BUF_SIZE]; //buffer
    //Getting text from keyboard
    for(;;) {
        bzero(input, BUF_SIZE+1);
        fgets(input, BUF_SIZE, stdin);
        strtok(input, "\n");
        if (strcmp(input, deactivate) == 0) {

            size_t k = strlen(input);
            char *id = malloc(k - sizeof(deactivate));
            strncpy(id, input + sizeof(deactivate), k - sizeof(deactivate));
            if (pClients[atoi(id)].isActive) {
                KickClient(atof(id));
                printf("Удалить клиента с ID %d.\n", atoi(id));
            } else {
                printf("Клиент с ID %d вышел из сети.\n", atoi(id));
            }
        }
        if (strcmp(input, shutdown_serv) == 0) {
            if (input[sizeof(shutdown_serv)-1] == '\0') {
                ShutdownRoullete();
                close(listener);
                printf("Все клиенты отключились.\nСокет сервера закрыт.");
                exit(0);
            } else
                printf("Не найдена команда '%s', вы имели в виду: команда '%s'\n", input, shutdown_serv);
        }
    }
}

int main(void){
    setlocale(LC_ALL, "Rus");
    pClients = (struct client *) malloc(MAX_GAMERS + 1);
    if (pClients == NULL) {
        perror("Не удалось выделить память.");
        exit(1);
    }

    int newfd;        // newly accepted socket descriptor
    socklen_t addrlen;
    struct sockaddr_in serv_addr, cli_addr;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("Ошибка при открытии слушающего сокета.");
        exit(1);
    }

    int enable = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) не удалось");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);


    /* Now bind the host address using bind() call.*/
    if (bind(listener, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    //thread for server
    pthread_t serv_thread;
    pthread_create(&serv_thread, &threadAttr, ServerHandler, NULL);
    listen(listener, 5);
    addrlen = sizeof cli_addr;
    for(;;) {
        newfd = accept(listener, (struct sockaddr *) &cli_addr, &addrlen);
        if (newfd < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        if (clients_count >= MAX_GAMERS && croupier_online) {
            SendToClient(newfd, "FULLTABLE");
            close(newfd);
            continue;
        } else {
            int cur_id = AppointmentId();
            pClients[cur_id].socket_desc = newfd;
            pClients[cur_id].bet = -1;

            printf("Сокет %d подключен\n", newfd);
            SendToClient(newfd, "CONNECTED");
            clients_count++;
            pthread_t client_thread;
            if (pthread_create(&client_thread, &threadAttr, ClientHandler, (void *)cur_id) != 0)
                perror("Creating thread false");
        }
    }
}
