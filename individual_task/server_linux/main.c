#include "users_and_data/work.h"
#include "sockets/socket.h"
#include <pthread.h>

#define BUFSIZE 8192

char* filename = "server_data.txt";
char buff[BUFSIZE];
struct prime_numbers data;

// Function for messaging with client
void* client_func(void* arg);

// Request handlers
void reg_request_handler(int sockfd, struct request req);
void login_request_handler(int sockfd, struct request req);
void del_request_handler(int sockfd, struct request req);
void maxprime_request_handler(int sockfd, struct request req);
void primes_request_handler(int sockfd, struct request req);
void range_request_handler(int sockfd, struct request req);
void calc_request_handler(int sockfd, struct request req);
void clear_request_handler(int sockfd, struct request req);
void quit_request_handler(int sockfd, struct request req);

// Function for sending response
void send_response(int sockfd, char* type, char* payload);

// Errors handler
int handle_errors(int sockfd, int error);
// Function for checking if request arguments are NULL
int check_arguments(int sockfd, struct request req);
// Function for checking if request token is NULL
int check_token(int sockfd, struct request req);

int main()
{

    int sockfd, newsockfd; // Socket for listening
    struct sockaddr_in cli_addr;
    unsigned int clilen;

    bzero(buff, BUFSIZE);

    // Get server data
    read_file(filename, buff, BUFSIZE);
    if (strlen(buff) != 0) retrieve_data(&data, buff);
    else clear_data(&data);
    bzero(buff, BUFSIZE);

    create_data_storage(); // Create data folders

    // Create listen socket
    sockfd = listen_socket();

    clilen = sizeof(cli_addr);

    while ((newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen))) {

        // Making new thread for messaging with client
        pthread_t client_thread; // Thread for messaging

        if (pthread_create(&client_thread, NULL, client_func,
                           &newsockfd) < 0) {
            close_socket(newsockfd, "Error while creating client thread");
            return 1;
        }
        pthread_detach(client_thread);
    }

    if (sockfd < 0){
        perror("Accept failed");
        return 1;
    }

    close_socket(sockfd, "Good Job!");

    sleep(1);
    return 0;
}

void* client_func(void* arg)
{
    int sockfd = *(int*)arg; // Client socket
    struct request req; // Request from client
    int res;

    for(;;) {

        res = get_request(sockfd, &req);
        // Handle errors
        if (handle_errors(sockfd, res))
            continue;

        // Handle request
        if (strcmp(req.comm.type, "REG") == 0) {
            reg_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "LOGIN") == 0) {
            login_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "DEL") == 0) {
            del_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "MAXPRIME") == 0) {
            maxprime_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "PRIMES") == 0) {
            primes_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "RANGE") == 0) {
            range_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "CALC") == 0) {
            calc_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "CLEAR") == 0) {
            clear_request_handler(sockfd, req);
        } else if (strcmp(req.comm.type, "QUIT") == 0) {
            quit_request_handler(sockfd, req);
            close_socket(sockfd, "Client is out");
            pthread_exit(0);
        } else {
            send_response(sockfd, RESPONSE_ERROR, "Unknown type of request");
        }
    }
}

// Function for handling reg request
void reg_request_handler(int sockfd, struct request req)
{
    // Check if arguments are NULL
    if (check_arguments(sockfd, req))
        return;

    // Check if user already exists
    int res;
    res = check_user(req.comm.arg1);

    // Handle errors
    if (res == OTHER_ERROR) {
        send_response(sockfd, RESPONSE_ERROR, "Internal error");
        return;
    }
    if (res != USER_NOT_FOUND) {
        send_response(sockfd, RESPONSE_ERROR, "User already exists");
        return;
    }

    // Create account for new client
    res = create_user(req.comm.arg1);
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
    res = check_user(req.comm.arg1);
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

// Function for deleting client
void del_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;
    // Delete data
    int res;
    char login[100];
    res = get_session_client(req.token, login);
    if (handle_errors(sockfd, res))
        return;
    res = delete_user(login);
    if (handle_errors(sockfd, res))
        return;

    // End session
    res = delete_session(req.token);
    if (handle_errors(sockfd, res))
        return;

    send_response(sockfd, RESPONSE_DELETED, "User data deleted");
}

//Function for getting the biggest calculated prime number
void maxprime_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    char max[20];
    int maxprime = find_maxprime(&data);

    sprintf(max, "%d", maxprime);

    send_response(sockfd, RESPONSE_OK, max);
}

//Function for getting a list of last N prime numbers
void primes_request_handler(int sockfd, struct request req)
{

    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    // Check if arguments are NULL
    if (check_arguments(sockfd, req))
        return;

    char primes[sizeof(data.primes)/sizeof(data.primes[0])] = {0};

    int res = get_list_of_primes(&data, primes, req.comm.arg1);

    if (handle_errors(sockfd, res))
        return;

    send_response(sockfd, RESPONSE_OK, primes);
}

//Function for getting a range for caclulations
void range_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    char range[20];

    sprintf(range, "%d", data.range);

    send_response(sockfd, RESPONSE_OK, range);
}

//Function for calculating prime numbers
void calc_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    check_range(&data, buff);

    int res = send(sockfd, buff, sizeof(buff), NULL);
    if (handle_errors(sockfd, res))
        return;

    bzero(buff, BUFSIZE);
    char calc_range[10] = {0};
    sprintf(calc_range, "%d", data.current_range);

    res = send(sockfd, calc_range, sizeof(calc_range), NULL);
    if (handle_errors(sockfd, res))
        return;

    char read_data[BUFSIZE] = {0};
    res = read_socket(sockfd, read_data, sizeof(read_data));
    if (handle_errors(sockfd, res))
        return;

    int cur_primes = 0;

    // Find current count of prime numbers
    for (int i = 0; i < (int)(sizeof(data.primes)/sizeof(data.primes[0])); i++) {
        if (data.primes[i] == 0) break;
        cur_primes++;
    }

    data.current_range++;
    char* ptr;
    char* prime = strtok(read_data, " ");
    int i = cur_primes;
    data.primes[i] = (int)strtol(prime, &ptr, 10);

    while (prime != NULL){
        i++;
        prime = strtok(NULL, " ");
        if (prime == NULL) break;
        data.primes[i] = (int)strtol(prime, &ptr, 10);
    }

    pack_data(&data, buff);
    write_file(filename, buff, sizeof(buff));
    bzero(buff, BUFSIZE);

    send_response(sockfd, RESPONSE_OK, "Prime numbers was calculated");
}

//Function for clearing all calculated prime numbers
void clear_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    clear_data(&data);

    send_response(sockfd, RESPONSE_OK, "Server data cleared");
}

// Function for handling quit request
void quit_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;
    // End session
    int res;
    res = delete_session(req.token);
    if (handle_errors(sockfd, res))
        return;

    // Send response that we have deleted token. Now user must delete his token
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
        case OPEN_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't open file");
            return 1;
        case READ_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't read from file");
            return 1;
        case WRITE_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't write to file");
            return 1;
        case CLOSE_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't close file");
            return 1;
        case BOUND_IS_NOT_MULTIPLE:
            send_response(sockfd, RESPONSE_ERROR, "Bound must be multiple to range or equal 0");
            return 1;
        case RANGE_IS_ALREADY_USED:
            send_response(sockfd, RESPONSE_ERROR, "Bound is already used");
            return 1;
        case NEGATIVE_BOUND:
            send_response(sockfd, RESPONSE_ERROR, "Bound can't be negative");
            return 1;
        case INCORRECT_BOUNDS:
            send_response(sockfd, RESPONSE_ERROR, "Upper bound must be larger than lower bound");
            return 1;
        case INCORRECT_COUNT:
            send_response(sockfd, RESPONSE_ERROR, "Count must be larger than 0");
            return 1;
        case OTHER_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Internal error");
            return 1;
        case READING_ERROR:
            close_socket(sockfd, "ERROR while reading from socket");
            pthread_exit(0);
        case WRITING_ERROR:
            close_socket(sockfd, "ERROR while writing to socket");
            pthread_exit(0);
        case READING_IS_NOT_FINISHED:
            close_socket(sockfd, "Client closed connection");
            pthread_exit(0);
        case REQUEST_LENGTH_ERROR:
            close_socket(sockfd, "ERROR in request length");
            pthread_exit(0);
        default:break;
    }
    return 0;
}

// Function for checking arguments
int check_arguments(int sockfd, struct request req)
{
    // Check if arguments are NULL

    if (req.comm.arg1 == NULL) {
            send_response(sockfd, RESPONSE_ERROR, "Illegal argument");
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
    return 0;
}

// Function for sending response to client
void send_response(int sockfd, char* type, char* payload)
{
    int res;
    res = response_request(sockfd, type, payload);
    if (res == WRITING_ERROR) {
        close_socket(sockfd, "ERROR writing to socket");
        pthread_exit(0);
    }
}