#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <locale.h>

#include <string.h>

#define BUF_SIZE 24
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#define ODD "38"
#define EVEN "37"

int money = 300;

char *compareBet(char* bet) {
    char *id_client, *bet_client, *sum_client;
    strtok(bet, " ");
    id_client = strtok(NULL, " ");
    bet_client = strtok(NULL, " ");
    sum_client = strtok(NULL, " ");
    if (strcmp(bet_client, ODD) == 0)
        bet_client = "Нечет";
    if (strcmp(bet_client, EVEN) == 0)
        bet_client = "Чет";
    if (strcmp(bet_client, "0") == 0)
        bet_client = "Зеро";
    char *result = (char *)malloc(18);
    sprintf(result, "| %s | %s | %s |", id_client, bet_client, sum_client);

    return result;
}

int displayBets(int socket) {
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE+1);

    write(socket, "GIVELIST", 8);
    printf("| ID | BET | SUM |\n");
    for(;;) {
        memset(buffer, 0, BUF_SIZE+1);

        if (recv(socket, buffer, BUF_SIZE, 0) < 0) {
            perror("Read error");
            return -1;
        }
        if (strcmp(buffer, "STOPLIST") == 0) {
            return 0;
        }
        char *res = compareBet(buffer);
        printf("%s\n", res);
    }
}

int waitStartGame (int socket, int bet_count, int sum) {
    char buffer[BUF_SIZE];
    printf("Ожидание результата...\n");
    memset(buffer, 0, BUF_SIZE+1);
    if (recv(socket, buffer, BUF_SIZE, 0) < 0) {
        return -1;
    }
    if (strstr(buffer, "RESULT") != NULL) {
        size_t k = strlen(buffer);
        char *bet = malloc(k - 7);
        strncpy(bet, buffer+7, k - 7);
        if (bet_count == atoi(bet) || ((atoi(bet) % 2 == 1) && bet_count == 38)
                || ((atoi(bet) % 2 == 0) && (atoi(bet) != 0) && bet_count == 37)) {
            printf("Вы выиграли.\n");
            if (bet_count == 38 || bet_count == 37) {
                printf("Ваш приз - %d.\n", sum * 2);
				money += sum * 2;
            } else {
                printf("Ваш приз - %d.\n", sum * 36);
				money += sum * 36;
            }
			printf("У Вас сейчас %d долларов.\n", money);
        } else {
            printf("Вы проиграли.\n");
        }
        return 0;
    }
        return -1;
}

int checkSumBet() {
    char buffer[BUF_SIZE];
    for (;;) {
        printf("Выберите сумму ставки (от 1 до 99)\n");
        bzero(buffer, BUF_SIZE + 1);
        fgets(buffer, BUF_SIZE, stdin);
        strtok(buffer, "\n");
        char number[3];
        char *ptr;
        strncpy(number, buffer, 3);
        long buf_int = strtol(number, &ptr, 10);
        if (ptr != number) {
            if (buf_int < 1 || buf_int > 99) {
                printf("Значение должно быть от 1 до 99.\n");
                continue;
            }
			if (buf_int > money) {
				printf("У Вас недостаточно денег. У Вас только %d долларов.\n", money);
				continue;
			}
            return (int) buf_int;
        } else {
            printf("Значение должно быть от 1 до 99.\n");
        }
    }
}

int sendBet(int socket, int bet) {
    char buffer[BUF_SIZE];
    int sum = checkSumBet();
    bzero(buffer, BUF_SIZE+1);
    sprintf(buffer, "%s %d %d", "MYBETIS", bet, sum);
	money -= sum;
	printf("У Вас сейчас %d долларов.\n", money);
    write(socket, buffer, 14);
    bzero(buffer, BUF_SIZE+1);
    read(socket, buffer, BUF_SIZE);
    if (strcmp(buffer, "WAITGAME") == 0) {
        if (waitStartGame(socket, bet, sum) < 0)
            return -1;
    }
    return 1;
}

int welcomeGamer(int socket) {
    char buffer[BUF_SIZE];
    char inside_buffer[BUF_SIZE];
    memset(inside_buffer, 0, BUF_SIZE+1);

    for(;;) {
        bzero(buffer, BUF_SIZE+1);
        read(socket, buffer, BUF_SIZE);
        if (strcmp(buffer, "GAMEPROCESS") == 0) {
            printf("Игра началась. Ожидание...\n");
            continue;
        }
        if (strcmp(buffer, "WELCOME") == 0) {
            for(;;) {
                printf("\nУ Вас сейчас %d долларов.\nВыберите действие:\n1.Просмотреть список ставок;\n2.Разместить ставку;\n3.Выйти;\n", money);
                memset(buffer, 0, BUF_SIZE+1);
                fgets(buffer, BUF_SIZE, stdin);
                strtok(buffer, "\n");
                if (strcmp(buffer, "1") == 0) {
                    if (displayBets(socket) < 0)
                        return -1;
                    break;
                }
                if (strcmp(buffer, "2") == 0) {
                    for(;;) {
                        printf("Выберите число от 0 до 36 или чет/нечет:\n");
                        memset(buffer, 0, BUF_SIZE+1);
                        fgets(inside_buffer, BUF_SIZE, stdin);
                        strtok(inside_buffer, "\n");

                        if (strcmp(inside_buffer, "чет") == 0) {
                            if (sendBet(socket, 37) < 0) {
                                return -1;
                            }
                            break;
                        }
                        if (strcmp(inside_buffer, "нечет") == 0) {
                            if (sendBet(socket, 38) < 0) {
                                return -1;
                            }
                            break;
                        }
                        char number[2];
                        char *ptr;
                        strncpy(number, inside_buffer, 2);
                        long buf_int = strtol(number, &ptr, 10);
                        if (ptr != number) {
                            if (buf_int < 0 || buf_int > 36) {
                                printf("Значение должно быть от 0 до 36.\n");
                                continue;
                            }
                            if (sendBet(socket, (int) buf_int) < 0) {
                                return -1;
                            }
                            break;
                        }
                    }
                    break;
                }
                if (strcmp(buffer, "3") == 0) {
                    bzero(buffer, BUF_SIZE+1);
                    write(socket, "EXIT", BUF_SIZE);
                    printf("До новых встреч!\n");
                    bzero(buffer, BUF_SIZE+1);
                    read(socket, buffer, BUF_SIZE);
                    if (strcmp(buffer, "KICKED") == 0) {
                        close(socket);
                        return -1;
                    } else {
                        printf("Работа сервера некорректна. Соединение разорвано.\n");
                        close(socket);
                        return -1;
                    }
                }
            }
        }

    }
}

int welcomeCroupier(int socket) {
    char buffer[BUF_SIZE];

    for(;;) {
        bzero(buffer, BUF_SIZE+1);
        read(socket, buffer, BUF_SIZE);
        if (strcmp(buffer, "GAMEPROCESS") == 0) {
            printf("Игра началась. Ожидание...\n");
            continue;
        }
        if (strcmp(buffer, "WELCOME") == 0) {
            for(;;) {
                printf("Выберите действие:\n1. Просмотреть список ставок;\n2. Начать игру;\n3. Выйти\n");
                memset(buffer, 0, BUF_SIZE+1);
                fgets(buffer, BUF_SIZE, stdin);
                strtok(buffer, "\n");
                if (strcmp(buffer, "1") == 0) {
                    if (displayBets(socket) < 0)
                        return -1;
                    break;
                }
                if (strcmp(buffer, "2") == 0) {
                    write(socket, "STARTGAME", BUF_SIZE);
                    printf("Игра началась. Ожидание результатов.\n");
                    bzero(buffer, BUF_SIZE+1);
                    read(socket, buffer, BUF_SIZE);
                    size_t k = strlen(buffer);
                    char *bet = malloc(k - 7);
                    strncpy(bet, buffer+7, k - 7);
                    printf("Выпавший номер на рулетке - %d.\n", atoi(bet));

                    break;
                }
                if (strcmp(buffer, "3") == 0) {
                    bzero(buffer, BUF_SIZE+1);
                    write(socket, "EXIT", BUF_SIZE);
                    printf("До новых встреч!\n");
                    bzero(buffer, BUF_SIZE+1);
                    read(socket, buffer, BUF_SIZE);
                    if (strcmp(buffer, "KICKED") == 0) {
                        close(socket);
                        return -1;
                    } else {
                        printf("Работа сервера некорректна. Соединение разорвано.\n");
                        close(socket);
                        return -1;
                    }
                }
            }
        }

    }
}

int gamerMode(int socket) {
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE+1);

    recv(socket, buffer, BUF_SIZE, 0);
    for(;;) {
        if (strcmp(buffer, "ONLYCROUP") == 0) {
            printf("Стол полон. Необходим крупье. Возвращайтесь позднее.\n");
            bzero(buffer, BUF_SIZE+1);
            read(socket, buffer, BUF_SIZE);
            if (strcmp(buffer, "KICKED") == 0) {
                close(socket);
                return -1;
            } else {
                printf("Работа сервера некорректна. Соединение разорвано.\n");
                close(socket);
                return -1;
            }
        }
        if (strstr(buffer, "GAMERID") != NULL) {
            size_t k = strlen(buffer);
            char *id = malloc(k - 8);
            strncpy(id, buffer+8, k - 8);
            printf("Приветствуем! Наше казино соблюдает анонимность пользователей. Ваш персональный ID '%s'\n", id);
            return welcomeGamer(socket);
        }
    }
}

int croupMode(int socket) {
    char buffer[BUF_SIZE];
    char buf[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE+1);

    recv(socket, buffer, BUF_SIZE, 0);
    for(;;) {
        if (strcmp(buffer, "CROUPEXIST") == 0) {
            printf("Крупье уже есть. Возвращайтесь позднее.\n");
            close(socket);
            return -1;
        }
        if (strcmp(buffer, "PASW") == 0) {
            printf("Пароль крупье: ");
            for(;;) {
                memset(buffer, 0, BUF_SIZE+1);
                fgets(buffer, BUF_SIZE, stdin);
                memset(buf, 0, BUF_SIZE+1);
                strcpy(buf, buffer);
                write(socket, buf, BUF_SIZE);
                bzero(buffer, BUF_SIZE+1);
                read(socket, buffer, BUF_SIZE);
                if (strcmp(buffer, "LOGGED") == 0) {
                    printf("Вход успешен.\n");
                    return welcomeCroupier(socket);
                }
                if (strcmp(buffer, "WRONGPASW") == 0) {
                    printf("Неправильный пароль.\nПовторите ввод: ");
                    continue;
                }
                if (strcmp(buffer, "THIRDPASW") == 0) {
                    printf("Неправильный пароль. Попробуйте позднее.\n");
                    return -1;
                }
            }
        }
    }
}

int chooseRole(int socket) {
    char buffer[BUF_SIZE]; //buffer
    for(;;) {
        printf("Выберите роль:\n1. Игрок\n2. Крупье\n");
        memset(buffer, 0, BUF_SIZE+1);
        fgets(buffer, BUF_SIZE, stdin);
        strtok(buffer, "\n");
        if (strcmp(buffer, "1") == 0) {
            write(socket, "GAMER", 5);
            return gamerMode(socket);
        }
        if (strcmp(buffer, "2") == 0) {
            write(socket, "CROUP", 5);
            return croupMode(socket);
        }
        printf("Роль не найдена.\n");
    }
}

int main(int argc, char *argv[]) {
	setlocale(0, "Rus");
    //WSADATA wsaData;
    int sockfd/*INVALID_SOCKET*/, iresult;
    struct addrinfo  *result = NULL, *ptr = NULL;
	struct addrinfo hints;

    char buffer[BUF_SIZE];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

	/*
    iresult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iresult != 0) {
        printf("WSAStartup failed with error: %d\n", iresult);
        return 1;
    }
	*/

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iresult = getaddrinfo(argv[1], argv[2], &hints, &result);
    if ( iresult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iresult);
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr=ptr->ai_next) {
        sockfd = (int) socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
	    printf("Извините, возникли ошибки:(");
            return 1;
        }

        iresult = connect(sockfd, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iresult == SOCKET_ERROR) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Добро пожаловать в Онлайн Казино:\n");
    memset(buffer, 0, BUF_SIZE+1);
    //fgets(buffer, BUF_SIZE, stdin);
    read(sockfd, buffer, BUF_SIZE);
    if (strcmp(buffer, "FULLTABLE") == 0) {
        printf("Простите, стол полон. Возвращайтесь позднее.\n");
        close(sockfd);
        exit(0);
    }
    if (strcmp(buffer, "CONNECTED") == 0) {
        bzero(buffer, BUF_SIZE+1);
        if (chooseRole(sockfd) < 0)
            exit(1);
    }
    return 0;
}
