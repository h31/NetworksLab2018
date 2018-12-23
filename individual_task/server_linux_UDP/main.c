#include "users_and_data/work.h"
#include "sockets/socket.h"
#include <pthread.h>

#define BUFSIZE 1024
#define RECV_SIZE 5000

char* filename = "server_data.txt";
char buff[BUFSIZE];
struct prime_numbers data;
struct sockaddr_in cli_addr;

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
void send_response(int sockfd, char* type, char* payload, struct request req);

// Errors handler
int handle_errors(int sockfd, int error, struct request req);
// Function for checking if request arguments are NULL
int check_arguments(int sockfd, struct request req);
// Function for checking if request token is NULL
int check_token(int sockfd, struct request req);

int main() {

    int sockfd; // Socket for listening
    unsigned int clilen;
    struct request req; // Request from client
    int res;

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
    memset((char *) &cli_addr, 0, clilen);

    for (;;) {

        res = get_request(sockfd, &req, &cli_addr);
        // Handle errors
        if (handle_errors(sockfd, res, req))
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
        } else {
            send_response(sockfd, RESPONSE_ERROR, "Unknown type of request", req);
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
        send_response(sockfd, RESPONSE_ERROR, "Internal error", req);
        return;
    }
    if (res != USER_NOT_FOUND) {
        send_response(sockfd, RESPONSE_ERROR, "User already exists", req);
        return;
    }

    // Create account for new client
    res = create_user(req.comm.arg1);
    if (handle_errors(sockfd, res, req))
        return;

    // Create new session for client
    char token[50];
    res = create_session(req.comm.arg1, token, 50);
    if (handle_errors(sockfd, res, req))
        return;

    // Return session token to client
    send_response(sockfd, RESPONSE_TOKEN, token, req);
}

// Function for handling login request
void login_request_handler(int sockfd, struct request req)
{
    // Check if arguments are NULL
    if (check_arguments(sockfd, req))
        return;

    // Delete previous session
    if (req.token != NULL) {
        delete_session(req.token);
    }

    // Check if user exists
    int res;
    res = check_user(req.comm.arg1);
    // Handle errors
    if (handle_errors(sockfd, res, req))
        return;

    // Login client and return session token
    char token[50];
    res = create_session(req.comm.arg1, token, 50);
    if (handle_errors(sockfd, res, req))
        return;

    send_response(sockfd, RESPONSE_TOKEN, token, req);
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
    if (handle_errors(sockfd, res, req))
        return;
    res = delete_user(login);
    if (handle_errors(sockfd, res, req))
        return;

    // End session
    res = delete_session(req.token);
    if (handle_errors(sockfd, res, req))
        return;

    send_response(sockfd, RESPONSE_DELETED, "User data deleted", req);
}

//Function for getting the biggest calculated prime number
void maxprime_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    char max[20];
    bzero(max, 20);

    int maxprime = find_maxprime(&data);

    sprintf(max, "%d", maxprime);

    send_response(sockfd, RESPONSE_OK, max, req);
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

    char primes[BUFSIZE];
    bzero(primes, BUFSIZE);

    int res = get_list_of_primes(&data, primes, req.comm.arg1);

    if (handle_errors(sockfd, res, req))
        return;

    send_response(sockfd, RESPONSE_OK, primes, req);
}

//Function for getting a range for caclulations
void range_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    char range[20];
    bzero(range, 20);

    sprintf(range, "%d", data.range);

    send_response(sockfd, RESPONSE_OK, range, req);
}

//Function for calculating prime numbers
void calc_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    check_range(&data, buff);

    int res = sendto(sockfd, buff, BUFSIZE, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
    if (handle_errors(sockfd, res, req))
        return;

    bzero(buff, BUFSIZE);
    char calc_range[20];
    bzero(calc_range, 20);
    sprintf(calc_range, "%d", data.current_range);

    res = sendto(sockfd, calc_range, 20, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
    if (handle_errors(sockfd, res, req))
        return;

    char range[20];
    bzero(range, 20);
    sprintf(range, "%d", data.range);

    res = sendto(sockfd, range, 20, 0, (struct sockaddr *) &cli_addr, sizeof(cli_addr));
    if (res < 0) {
        handle_errors(sockfd, WRITING_ERROR, req);
        return;
    }

    char* read_data;
    read_data = (char *) calloc(RECV_SIZE, sizeof(char));
    bzero(read_data, RECV_SIZE);

    res = read_socket(sockfd, read_data, RECV_SIZE, &cli_addr);
    if (handle_errors(sockfd, res, req))
        return;

    char* ptr;
    data.current_range++;
    char* prime = strtok(read_data, " ");
    int i = 0;
    data.primes[i] = (int)strtol(prime, &ptr, 10);

    while (prime != NULL){
        i++;
        prime = strtok(NULL, " ");
        if (prime == NULL) break;
        data.primes[i] = (int)strtol(prime, &ptr, 10);
    }

    bzero(read_data, BUFSIZE);
    pack_data(&data, read_data);
    write_file(filename, read_data, 65536);
    bzero(read_data, BUFSIZE);

    send_response(sockfd, RESPONSE_OK, "Prime numbers was calculated", req);
}

//Function for clearing all calculated prime numbers
void clear_request_handler(int sockfd, struct request req)
{
    // Check if token is NULL
    if (check_token(sockfd, req))
        return;

    clear_data(&data);

    send_response(sockfd, RESPONSE_OK, "Server data cleared", req);
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
    if (handle_errors(sockfd, res, req))
        return;

    // Send response that we have deleted token. Now user must delete his token
    send_response(sockfd, RESPONSE_DELETED, "Token deleted", req);
}

// Function for handling errors
int handle_errors(int sockfd, int error, struct request req)
{
    switch (error) {
        case OK:
            return 0;
        case USER_ALREADY_EXISTS:
            send_response(sockfd, RESPONSE_ERROR, "User already exists", req);
            return 1;
        case USER_NOT_FOUND:
            send_response(sockfd, RESPONSE_ERROR, "User not found", req);
            return 1;
        case OPEN_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't open file", req);
            return 1;
        case READ_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't read from file", req);
            return 1;
        case WRITE_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't write to file", req);
            return 1;
        case CLOSE_FILE_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Can't close file", req);
            return 1;
        case BOUND_IS_NOT_MULTIPLE:
            send_response(sockfd, RESPONSE_ERROR, "Bound must be multiple to range or equal 0", req);
            return 1;
        case RANGE_IS_ALREADY_USED:
            send_response(sockfd, RESPONSE_ERROR, "Bound is already used", req);
            return 1;
        case NEGATIVE_BOUND:
            send_response(sockfd, RESPONSE_ERROR, "Bound can't be negative", req);
            return 1;
        case INCORRECT_BOUNDS:
            send_response(sockfd, RESPONSE_ERROR, "Upper bound must be larger than lower bound", req);
            return 1;
        case INCORRECT_COUNT:
            send_response(sockfd, RESPONSE_ERROR, "Count must be larger than 0", req);
            return 1;
        case OTHER_ERROR:
            send_response(sockfd, RESPONSE_ERROR, "Internal error", req);
            return 1;
        case LOST_OR_WRONG_PACKET:
            send_response(sockfd, RESPONSE_ERROR, "Packet lost or wrong. Send again", req);
            return 1;
        case READING_ERROR:
            close_socket(sockfd, "ERROR while reading from socket");
        case WRITING_ERROR:
            close_socket(sockfd, "ERROR while writing to socket");
        case READING_IS_NOT_FINISHED:
            close_socket(sockfd, "Client closed connection");
        default:break;
    }
    return 0;
}

// Function for checking arguments
int check_arguments(int sockfd, struct request req)
{
    // Check if arguments are NULL

    if (req.comm.arg1 == NULL) {
            send_response(sockfd, RESPONSE_ERROR, "Illegal argument", req);
            return 1;
    }
    return 0;
}

// Function for checking token
int check_token(int sockfd, struct request req)
{
    // Check if token is NULL
    if (req.token == NULL) {
        send_response(sockfd, RESPONSE_ERROR, "You are not logged", req);
        return 1;
    }
    return 0;
}

// Function for sending response to client
void send_response(int sockfd, char* type, char* payload, struct request req)
{
    int res;
    res = response_request(sockfd, type, payload, &cli_addr, req.index);
    if (res == WRITING_ERROR) {
        close_socket(sockfd, "ERROR writing to socket");
    }
}