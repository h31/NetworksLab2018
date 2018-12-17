#include "sockets/sockets.h"

int main(int argc, char* argv[])
{
    struct request req; // Request to server
    struct response resp; // Response from server
    int sockfd; // Socket for connection to server
    int res;
    char buf[256]; // Input buffer
    char token[50]; // Token for session
    char login[50]; // Client login

    // Clear token
    bzero(token, 50);

    // Clear login
    bzero(login, 50);

    // Connect to server
    sockfd = connect_to(argc, argv);
    if (sockfd < 0) {
        exit(1);
    }
    printf("Connected to server with fd=%d\n", sockfd);

    while (1) {
        // Print console text
        if (strlen(login) == 0) {
            strcpy(login, "not-logged");
        }
        printf("%s:command>", login);

        // Get string from client
        bzero(buf, 256);
        fgets(buf, 255, stdin);
        res = parse_request(buf, &req);

        // Handle errors
        if (res == INVALID_TYPE) {
            printf("Invalid type of request!\n");
            break;
        }

        // Show help message
        if (strcmp(req.comm.type, "HELP") == 0) {
            printf("Command list:\n");
            printf("REG <login> <password> - зарегистрировать нового клиента. После регистрации сервер автоматически залогинит пользователя в созданном аккаунте.\n");
            printf("LOGIN <login> <password> - войти в аккаунт.\n");
            printf("GET - получить состояние счета клиента. Доступно только после входа в аккаунт.\n");
            printf("SEND <amount> <login> - переслать заданную сумму пользователю с указанным логином.\n");
            printf("DEL - удалить текущий аккаунт.\n");
            printf("QUIT - выйти из приложения.\n");
            printf("HELP - вывести справку по командам.\n");
            continue;
        }

        // Set token
        req.token = token;

        // Send request
        res = send_request(sockfd, &req);
        if (res != 0) {
            break;
        }

        // Read response
        res = get_response(sockfd, &resp);
        if (res != 0) {
            break;
        }

        // Hadnle response
        if (strcmp(resp.type, RESPONSE_TOKEN) == 0) {
            strcpy(token, resp.payload);
            strcpy(login, req.comm.arg1);
            printf("You are logged as %s\n", login);
        }

        if (strcmp(resp.type, RESPONSE_DELETED) == 0) {
            bzero(token, 50);
            printf("Session is over. Server response:%s\n", resp.payload);
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
    printf("Now close socket\n");
    close_socket(sockfd, "no errors");
    return 0;
}
