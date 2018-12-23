#include "sockets/socket.h"
#include "data/data.h"

int main(int argc, char* argv[])
{
    struct request req; // Request to server
    struct response resp; // Response from server
    int sockfd; // Socket for connection to server
    int res;
    char buf[256]; // Input buffer
    char mes_buf[BUFSIZE]; // Message from server
    char size[10]; // Current range for calculation
    char range[10]; // Server range
    char send_data[BUFSIZE]; // Data sent to server
    char token[20]; // Token for session
    char login[20]; // Client login

    // Clear token
    bzero(token, 20);

    // Clear login
    bzero(login, 20);

    // Clear message
    bzero(mes_buf, BUFSIZE);
    bzero(size, sizeof(size));
    bzero(range, sizeof(range));
    bzero(send_data, BUFSIZE);

    // Connect to server
    sockfd = connect_socket(argc, argv);
    if (sockfd < 0) {
        exit(1);
    }
    printf("Connected to server with fd=%d\n", sockfd);

    while (1) {
        // Print console text
        if (strlen(login) == 0) {
            strcpy(login, "new-user");
        }
        printf("%s:command>", login);

        // Get string from client
        bzero(buf, 256);
        fgets(buf, 255, stdin);
        res = parse_request(buf, &req);

        // Handle errors
        if (res == INVALID_REQUEST) {
            printf("Invalid type of request!\n");
            break;
        }

        // Show help message
        if (strcmp(req.comm.type, "HELP") == 0) {
            printf("Command list:\n");
            printf("REG <login> - зарегистрировать нового клиента. После регистрации сервер автоматически залогинит пользователя в созданном аккаунте.\n");
            printf("LOGIN <login> - войти в аккаунт.\n");
            printf("DEL - отключение выбранного клиента\n");
            printf("MAXPRIME- получить максимальное вычисленное простое число.\n");
            printf("PRIMES <N> - получение последних N рассчитанных простых чисел.\n");
            printf("RANGE - получение диапазона расчёта простых чисел.\n");
            printf("CALC - расчёт простых чисел.\n");
            printf("CLEAR - очистка данных обо всех простых числах.\n");
            printf("QUIT - выйти из приложения.\n");
            printf("HELP - вывести справку по командам.\n");
            continue;
        }

        // Set token
        req.token = token;

        // Send request
        res = send_request(sockfd, &req);
        if (res < 0) {
            break;
        }

        if (strcmp(req.comm.type, "CALC") == 0 && strlen(req.token) != 0){
            char* ptr;
            res = read_socket(sockfd, mes_buf, BUFSIZE);
            if (res < 0) {
                break;
            }
            strcat(mes_buf, "\n");
            printf(mes_buf);
            bzero(mes_buf, BUFSIZE);

            res = read_socket(sockfd, size, 20);
            if (res < 0) {
                break;
            }
            int current_range = (int)strtol(size, &ptr, 10);
            bzero(size, 20);

            char tmp[20];
            memcpy(tmp, token, 20);
            res = read_socket(sockfd, range, 20);
            if (res < 0) {
                break;
            }

            int calc_range = (int)strtol(range, &ptr, 10);
            bzero(range, 20);

            int serv_data[SEND_SIZE];
            bzero(serv_data, SEND_SIZE);

            res = calculate_data(serv_data, current_range, calc_range);
            if (res < 0) {
                break;
            }

            pack_data(serv_data, calc_range, send_data);
            res = send(sockfd, send_data, 5000, NULL);
            if (res < 0) {
                break;
            }

            memcpy(token, tmp, 20);
            bzero(serv_data, SEND_SIZE);
        }

        // Read response
        res = response(sockfd, &resp);
        if (res < 0) {
            break;
        }

        // Handle response
        if (strcmp(resp.type, RESPONSE_TOKEN) == 0) {
            strcpy(token, resp.payload);
            strcpy(login, req.comm.arg1);
            printf("You are logged as %s\n", login);
        }

        if (strcmp(resp.type, RESPONSE_DELETED) == 0) {
            bzero(token, 50);
            printf("Session is over\n");
            strcpy(login, "not-logged");
        }

        if (strcmp(resp.type, RESPONSE_OK) == 0) {
            printf("%s\n", resp.payload);
        }

        if (strcmp(resp.type, RESPONSE_ERROR) == 0) {
            printf("ERROR: %s\n", resp.payload);
        }

        if (strcmp(req.comm.type, "QUIT") == 0) {
            break;
        }

    }

    close_socket(sockfd, "Socket closed");
    return 0;
}