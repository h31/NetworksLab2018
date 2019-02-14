#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>

#define BUF_SIZE 24

#define ODD "38"
#define EVEN "37"

char *compareBet(char* bet) {
    char *id_client, *bet_client, *sum_client;
    strtok(bet, " ");
    id_client = strtok(NULL, " ");
    bet_client = strtok(NULL, " ");
    sum_client = strtok(NULL, " ");
    if (strcmp(bet_client, ODD) == 0)
        bet_client = "Odd";
    if (strcmp(bet_client, EVEN) == 0)
        bet_client = "Even";
    if (strcmp(bet_client, "0") == 0)
        bet_client = "Zero";
    char *result = malloc(18);
    sprintf(result, "| %s | %s | %s |", id_client, bet_client, sum_client);

    return result;
}

int displayBets(int socket) {
    char buffer[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE+1);

    write(socket, "GIVELIST", 8);
    printf("| ID | BET  | SUM |\n");
    for(;;) {
        memset(buffer, 0, BUF_SIZE+1);

        if (recv((SOCKET) socket, buffer, BUF_SIZE, 0) < 0) {
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
    printf("Waiting result.\n");
    memset(buffer, 0, BUF_SIZE+1);
    if (recv((SOCKET) socket, buffer, BUF_SIZE, 0) < 0) {
        return -1;
    }
    if (strstr(buffer, "RESULT") != NULL) {
        size_t k = strlen(buffer);
        char *bet = malloc(k - 7);
        strncpy(bet, buffer+7, k - 7);
        if (bet_count == atoi(bet) || ((atoi(bet) % 2 == 1) && bet_count == 38)
                || ((atoi(bet) % 2 == 0) && (atoi(bet) != 0) && bet_count == 37)) {
            printf("You won.\n");
            if (bet_count == 38 || bet_count == 37) {
                printf("Your prize is %d.\n", sum * 2);
            } else {
                printf("Your prize is %d.\n", sum * 36);
            }
        } else {
            printf("You lose.\n");
        }
        return 0;
    }
        return -1;
}

int checkSumBet() {
    char buffer[BUF_SIZE];
    for (;;) {
        printf("Choose sum for your bet (from 1 to 100)\n");
        bzero(buffer, BUF_SIZE + 1);
        fgets(buffer, BUF_SIZE, stdin);
        strtok(buffer, "\n");
        char number[3];
        char *ptr;
        strncpy(number, buffer, 3);
        long buf_int = strtol(number, &ptr, 10);
        if (ptr != number) {
            if (buf_int < 1 || buf_int > 100) {
                printf("Number must be from 1 to 100\n");
                continue;
            }
            return (int) buf_int;
        } else {
            printf("Number must be from 1 to 100\n");
        }
    }
}

int sendBet(int socket, int bet) {
    char buffer[BUF_SIZE];
    int sum = checkSumBet();
    bzero(buffer, BUF_SIZE+1);
    sprintf(buffer, "%s %d %d", "MYBETIS", bet, sum);
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
            printf("Game in process. Waiting...\n");
            continue;
        }
        if (strcmp(buffer, "WELCOME") == 0) {
            for(;;) {
                printf("Choose Action (type number):\n1.View list of bets;\n2.Place bet;\n3.Exit;\n");
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
                        printf("Choose number from 0 to 36 or Even/Odd:\n");
                        memset(buffer, 0, BUF_SIZE+1);
                        fgets(inside_buffer, BUF_SIZE, stdin);
                        strtok(inside_buffer, "\n");

                        if (strcmp(inside_buffer, "Even") == 0) {
                            if (sendBet(socket, 37) < 0) {
                                return -1;
                            }
                            break;
                        }
                        if (strcmp(inside_buffer, "Odd") == 0) {
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
                                printf("Number must be from 0 to 36\n");
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
                    printf("See you later. Bye-Bye!\n");
                    bzero(buffer, BUF_SIZE+1);
                    read(socket, buffer, BUF_SIZE);
                    if (strcmp(buffer, "KICKED") == 0) {
                        close(socket);
                        return -1;
                    } else {
                        printf("The server does not work correctly. The connection is terminated.\n");
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
            printf("Game in process. Waiting...\n");
            continue;
        }
        if (strcmp(buffer, "WELCOME") == 0) {
            for(;;) {
                printf("Choose Action (type number):\n1.View list of bets;\n2.Start game;\n3.Exit\n");
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
                    printf("Game is start. Wait result\n");
                    bzero(buffer, BUF_SIZE+1);
                    read(socket, buffer, BUF_SIZE);
                    size_t k = strlen(buffer);
                    char *bet = malloc(k - 7);
                    strncpy(bet, buffer+7, k - 7);
                    printf("Roulette win number - %d.\n", atoi(bet));

                    break;
                }
                if (strcmp(buffer, "3") == 0) {
                    bzero(buffer, BUF_SIZE+1);
                    write(socket, "EXIT", BUF_SIZE);
                    printf("See you later. Bye-Bye!\n");
                    bzero(buffer, BUF_SIZE+1);
                    read(socket, buffer, BUF_SIZE);
                    if (strcmp(buffer, "KICKED") == 0) {
                        close(socket);
                        return -1;
                    } else {
                        printf("The server does not work correctly. The connection is terminated.\n");
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

    recv((SOCKET) socket, buffer, BUF_SIZE, 0);
    for(;;) {
        if (strcmp(buffer, "ONLYCROUP") == 0) {
            printf("Table is full. We need croupier. Come back later.\n");
            bzero(buffer, BUF_SIZE+1);
            read(socket, buffer, BUF_SIZE);
            if (strcmp(buffer, "KICKED") == 0) {
                close(socket);
                return -1;
            } else {
                printf("The server does not work correctly. The connection is terminated.\n");
                close(socket);
                return -1;
            }
        }
        if (strstr(buffer, "GAMERID") != NULL) {
            size_t k = strlen(buffer);
            char *id = malloc(k - 8);
            strncpy(id, buffer+8, k - 8);
            printf("Hi! Our Roulette for anonymous users. Your personal identification '%s'\n", id);
            return welcomeGamer(socket);
        }
    }
}

int croupMode(int socket) {
    char buffer[BUF_SIZE];
    char buf[BUF_SIZE];
    memset(buffer, 0, BUF_SIZE+1);

    recv((SOCKET) socket, buffer, BUF_SIZE, 0);
    for(;;) {
        if (strcmp(buffer, "CROUPEXIST") == 0) {
            printf("Croupier already exist. Come back later.\n");
            close(socket);
            return -1;
        }
        if (strcmp(buffer, "PASW") == 0) {
            printf("Password croupier: ");
            for(;;) {
                memset(buffer, 0, BUF_SIZE+1);
                fgets(buffer, BUF_SIZE, stdin);
                memset(buf, 0, BUF_SIZE+1);
                strcpy(buf, buffer);
                write(socket, buf, BUF_SIZE);
                bzero(buffer, BUF_SIZE+1);
                read(socket, buffer, BUF_SIZE);
                if (strcmp(buffer, "LOGGED") == 0) {
                    printf("Login is successful.\n");
                    return welcomeCroupier(socket);
                }
                if (strcmp(buffer, "WRONGPASW") == 0) {
                    printf("Password is wrong:\nRetry: ");
                    continue;
                }
                if (strcmp(buffer, "THIRDPASW") == 0) {
                    printf("Password is wrong. Sorry, comeback later.\n");
                    return -1;
                }
            }
        }
    }
}

int chooseRole(int socket) {
    char buffer[BUF_SIZE]; //buffer
    for(;;) {
        printf("Choose role (type number):\n1. Gamer\n2. Croupier\n");
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
        printf("No role found\n");
    }
}

int main(int argc, char *argv[]) {
    WSADATA wsaData;
    int sockfd = INVALID_SOCKET, iresult;
    struct addrinfo  *result = NULL, *ptr = NULL, hints;

    char buffer[BUF_SIZE];

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    iresult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iresult != 0) {
        printf("WSAStartup failed with error: %d\n", iresult);
        return 1;
    }

    memset( &hints, 0, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iresult = getaddrinfo(argv[1], argv[2], &hints, &result);
    if ( iresult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iresult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr=ptr->ai_next) {
        sockfd = (int) socket(ptr->ai_family, ptr->ai_socktype,
                              ptr->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", (long) WSAGetLastError());
            WSACleanup();
            return 1;
        }

        iresult = connect((SOCKET) sockfd, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iresult == SOCKET_ERROR) {
            closesocket((SOCKET) sockfd);
            sockfd = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (sockfd == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Welcome to Azino777:\n");
    memset(buffer, 0, BUF_SIZE+1);
    //fgets(buffer, BUF_SIZE, stdin);
    read(sockfd, buffer, BUF_SIZE);
    if (strcmp(buffer, "FULLTABLE") == 0) {
        printf("Sorry, table is full. Come back later\n");
        close(sockfd);
        WSACleanup();
        exit(0);
    }
    if (strcmp(buffer, "CONNECTED") == 0) {
        bzero(buffer, BUF_SIZE+1);
        if (chooseRole(sockfd) < 0)
            exit(1);
    }
    return 0;
}