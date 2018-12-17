#include "files/files.h"
#include "logging/logging.h"
#include "sockets/sockets.h"

int console_log = 1;

FILE* file_to_log;

// Thread functions
// Function for listening 5001 port and accepting connections
void* listen_func(void* arg);

// Function for messaging with client
void* client_func(void* arg);

// Request handlers
void get_request_handler(int sockfd, struct request req);
void reg_requesr_handler(int sockfd, struct request req);
void del_request_handler(int sockfd, struct request req);
void list_request_handler(int sockfd, struct request req);
void send_request_handler(int sockfd, struct request req);
void login_reqiest_handler(int sockfd, struct request req);
void quit_request_handler(int sockfd, struct request req);

// Funciton for sending response
void send_response(int sockfd, char* type, char* payload);

// Free allocated for request memory
void free_mem(struct request req);

// Errors handler
int handle_errors(int sockfd, int error);
// Function for checking if request arguments are NULL
int check_arguments(int sockfd, struct request req);
// Function for checking if request token is NULL
int check_token(int sockfd, struct request req);

int main()
{
    pthread_t listen_thread; // Thread for listening
    int listen_socket; // Socket for listening
    int res; // Result of functions
    //char key; // Key pressed
    char input_buf[256]; // Input buffer

    log_init("server.log");
    init_data_folders(); // Create data folders
    mlog("Staring server");

    // Create listen socket
    listen_socket = create_listen_socket();
    mlogf("listen socket fd = %d", listen_socket);
    // Create thread for listening
    res = pthread_create(&listen_thread, NULL, listen_func, &listen_socket);
    if (res != 0) {
        close_socket(listen_socket, "ERROR while creating listen thread");
    }

    while (1) {
        //key = getchar();
        bzero(input_buf, 256);
        fgets(input_buf, 255, stdin);
        if(strcmp(input_buf, "l\n") == 0) {
        	console_log = !console_log;
        }
        else if(strcmp(input_buf, "q\n") == 0) {
        	mlog("Shutting down server");
            break;
        }
        else if(strcmp(input_buf, "list\n") == 0) {
        	mlog("Logged clients:");
        	char list[1024];
   			list_all_sessions(list, 1024);
   			mlogf("%s", list);
        }
        else if(strcmp(input_buf, "help\n") == 0) {
        	printf("Commands:\n");
        	printf("q - quit\n");
        	printf("l - switch on/off logging to console\n");
        	printf("list - list all logged clients and their tokens\n");
        	printf("del <token> - delete session by token\n");
        }
        else {
        	char* str_token;
        	str_token = strtok(input_buf, " \n");
        	if(strcmp(str_token, "del") != 0) {
        		mlog("Wrong command");
        		continue;
        	}
        	str_token = strtok(NULL, " \n");
        	if (str_token == NULL) {
        		mlog("Argument: client login");
        		continue;
        	}
        	res = delete_session(str_token);
        }
        /*switch (key) {
        case 'l': // Turn on/off logging to console
            console_log = !console_log;
            break;
        case 'q': // Quit
            mlog("Shutting down server");
            break;
        case 'c':
        	mlog("Logged clients:");
        	char list[1024];
   			list_all_sessions(list, 1024);
   			mlogf("%s", list);
   			break;
        }*/
    }
    close_socket(listen_socket, "no errors");
    system("rm -rf data/session/*");
    log_close();
    sleep(1);
    return 0;
}

void* listen_func(void* arg)
{
    int sockfd = *(int*)arg; // Listen socket
    int newsockfd; // Accepted socket for client

    struct sockaddr_in cli_addr;
    unsigned int clilen = sizeof(cli_addr);

    // Accept connections
    while (1) {
        mlog("Waiting for connections");
        // Accept new connection
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if (newsockfd < 0) {
            mlog("ERROR on accept");
            pthread_exit(1);
        }

        mlogf("New connection accepted with fd = %d", newsockfd);

        // Making new thread for messaging with client
        pthread_t client_thread; // Thread for messaging
        int result; // Result of thread creating
        result = pthread_create(&client_thread, NULL, client_func,
            newsockfd); // Create new thread
        if (result != 0) {
            close_socket(newsockfd, "Error while creating client thread");
        }
        //char test[200];
        //sprintf(test, "pstree -cap > sockfd%d.txt", newsockfd);
        //system(test);
    }
    pthread_exit(0);
}

void* client_func(void* arg)
{
    int sockfd = (int)arg; // Client socket

    // Get port number
    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getpeername(sockfd, (struct sockaddr*)&sin, &len) == -1)
        close_socket(sockfd, "ERROR on getpeername");
    else
        mlogf("Starting client socket with fd=%d, port number= %d", sockfd, ntohs(sin.sin_port));

    int res = 0;

    while (1) {
        struct request req; // Request from client
        mlog("Waiting for request");
        res = get_request(sockfd, &req);

        // Handle errors
        if (handle_errors(sockfd, res)) {
            free_mem(req);
            continue;
        }

        // Log request
        mlogf("REQUEST:\nType: %s\nArg1: %s\nArg2: %s\nToken: %s", req.comm.type, req.comm.arg1, req.comm.arg2, req.token);

        // Handle request
        if (strcmp(req.comm.type, "GET") == 0) {
            get_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "REG") == 0) {
            reg_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "LOGIN") == 0) {
            login_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "LIST") == 0) {
            list_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "SEND") == 0) {
            send_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "DEL") == 0) {
            del_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "QUIT") == 0) {
            quit_request_handler(sockfd, req);
        } else {
            mlogf("Unknow type of request: %s", req.comm.type);
            send_response(sockfd, RESPONSE_ERROR, "Unknown type of request");
        }

        // Free memory
        free_mem(req);
    }
    close_socket(sockfd, "no errors");
    pthread_exit(0);
}

// Function for handling get request
void get_request_handler(int sockfd, struct request req)
{
    if (check_token(sockfd, req))
        return;

    int res;
    char login[1];
    int cash;
    char cash_string[20];
    char password[128];

    res = get_session_client(req.token, login);
    if (handle_errors(sockfd, res))
        return;

    res = get_account_data(login, &cash, password);
    if (handle_errors(sockfd, res))
        return;

    sprintf(cash_string, "%d", cash);

    send_response(sockfd, RESPONSE_OK, cash_string);
}

// Function for handling reg request
void reg_request_handler(int sockfd, struct request req)
{
    // Check if arguments are NULL
    if (check_arguments(sockfd, req))
        return;

    // Check if user already exists
    int res;
    res = check_account(req.comm.arg1, req.comm.arg2);

    // Handle errors
    if (res == ERROR) {
        send_response(sockfd, RESPONSE_ERROR, "Internal error");
        return;
    }
    if (res != USER_NOT_FOUND) {
        send_response(sockfd, RESPONSE_ERROR, "User already exists");
        return;
    }

    // Create account for new client
    res = create_account(req.comm.arg1, req.comm.arg2);
    if (handle_errors(sockfd, res))
        return;

    // Create new session for client
    char token[50];
    res = create_session(req.comm.arg1, token, 50);
    if (handle_errors(sockfd, res))
        return;

    // Return session token to client
    send_response(sockfd, RESPONSE_TOKEN, token);
}

// Function for handling login request
void login_request_handler(int sockfd, struct request req)
{
    // Check if arguments are NULL
    if (check_arguments(sockfd, req))
        return;

    // Delete prevous session
    if (req.token != NULL) {
        delete_session(req.token);
    }

    // Check if user exists
    int res;
    res = check_account(req.comm.arg1, req.comm.arg2);
    // Handle errors
    if (handle_errors(sockfd, res))
        return;

    // Login client and return session token
    char token[50];
    res = create_session(req.comm.arg1, token, 50);
    if (handle_errors(sockfd, res))
        return;

    send_response(sockfd, RESPONSE_TOKEN, token);
}

// Function for handling list request
void list_request_handler(int sockfd, struct request req)
{
    // LIST REQUEST
    char list[1024];
    list_all_accounts(list, 1024);
    send_response(sockfd, RESPONSE_OK, list);
}

// Function for handling send request
// Command: arg1 - money to send, arg2 - receiver
void send_request_handler(int sockfd, struct request req)
{
    // Check if arguments are NULL
    if (check_arguments(sockfd, req))
        return;
    if (check_token(sockfd, req))
        return;

    int res;
    char login[128];
    char password[128];
    int sender_cash;
    int receiver_cash;
    int send_cash;

    // Get receiver cash
    res = get_account_data(req.comm.arg2, &receiver_cash, password);
    if (handle_errors(sockfd, res))
        return;

    // Get account cash
    res = get_session_client(req.token, login);
    if (handle_errors(sockfd, res))
        return;

    res = get_account_data(login, &sender_cash, password);
    if (handle_errors(sockfd, res))
        return;

    // Convert arg to send_cash
    res = sscanf(req.comm.arg1, "%d", &send_cash);
    if (sender_cash < send_cash) {
        handle_errors(sockfd, NOT_ENOUGH_CASH);
        return;
    }

    // Get money from client
    res = withdraw_account_cash(login, send_cash);
    if (handle_errors(sockfd, res))
        return;

    // Send money to other client
    res = put_account_cash(req.comm.arg2, send_cash);
    if (handle_errors(sockfd, res))
        return;

    send_response(sockfd, RESPONSE_OK, "Success");
}

// Function for handling del request
void del_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;
    // Delete data
    int res;
    char login[1];
    res = get_session_client(req.token, login);
    if (handle_errors(sockfd, res))
        return;
    res = delete_account(login);
    if (handle_errors(sockfd, res))
        return;

    // End session
    res = delete_session(req.token);
    if (handle_errors(sockfd, res))
        return;

    send_response(sockfd, RESPONSE_DELETED, "User data deleted");
}

// Function for handling quit request
void quit_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req)) {
        return;
    }
    // End session
    int res;
    res = delete_session(req.token);
    if (handle_errors(sockfd, res))
        return;

    // Send responce that we have deleted token. Now user must delete his token
    send_response(sockfd, RESPONSE_DELETED, "Token deleted");
}

// Function for handling errors
int handle_errors(int sockfd, int error)
{
    switch (error) {
    case OK:
        return 0;
    case USER_ALREADY_EXISTS:
        send_response(sockfd, RESPONSE_ERROR, "User already exists");
        return 1;
    case USER_NOT_FOUND:
        send_response(sockfd, RESPONSE_ERROR, "User not found");
        return 1;
    case WRONG_PASSWORD:
        send_response(sockfd, RESPONSE_ERROR, "Wrong password");
        return 1;
    case NOT_ENOUGH_CASH:
        send_response(sockfd, RESPONSE_ERROR, "Insufficient funds to complete the transaction");
        return 1;
    case ERROR:
        send_response(sockfd, RESPONSE_ERROR, "Internal error");
        return 1;
    case ERROR_READING_FROM_SOCKET:
        close_socket(sockfd, "ERROR while reading from socket");
        pthread_exit(1);
        break;
    case READING_IS_NOT_FINISHED:
        close_socket(sockfd, "Client closed connection");
        pthread_exit(1);
        break;
    case REQUEST_LENGTH_ERROR:
        close_socket(sockfd, "ERROR in request length");
        pthread_exit(1);
        break;
    }
}

void free_mem(struct request req)
{
    mlog("Start free request memory");
    free(req.comm.type);
    if (req.comm.arg1 != NULL)
        free(req.comm.arg1);
    if (req.comm.arg2 != NULL)
        free(req.comm.arg2);
    if (req.token != NULL)
        free(req.token);
    mlog("Free request memory is done");
}

// Function for checking arguments
int check_arguments(int sockfd, struct request req)
{
    // Check if arguments are NULL
    if (req.comm.arg1 == NULL || req.comm.arg2 == NULL) {
        mlog("ERROR. Illegal arguments");
        send_response(sockfd, RESPONSE_ERROR, "Illegal arguments");
        return 1;
    }
    return 0;
}

// Function for checking token
int check_token(int sockfd, struct request req)
{
    // Check if token is NULL
    if (req.token == NULL) {
        send_response(sockfd, RESPONSE_ERROR, "You are not logged");
        return 1;
    }
    char* login;
    int res = get_session_client(req.token, login);
    if(res != 0 || login == NULL) {
    	mlog("ERROR!");
    	send_response(sockfd, RESPONSE_DELETED, "You have been disconnected");
    	return 1;
    }
    return 0;
}

// Function for sending response to client
void send_response(int sockfd, char* type, char* payload)
{
    // Log response
    mlogf("RESPONSE:\nType: %s\nPayload: %s", type, payload);
    int res;
    res = internal_send_response(sockfd, type, payload);
    if (res == ERROR_WRITING_TO_SOCKET) {
        close_socket(sockfd, "ERROR writing to socket");
        pthread_exit(1);
    }
}
