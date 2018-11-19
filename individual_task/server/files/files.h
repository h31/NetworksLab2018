/**
 * Client data is stored in folder "data/clients"
 * Tokens of logged clients are stored in folder "data/tokens"
 */
#include "../logging/logging.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FILES_OK 0
#define USER_ALREADY_EXISTS 201
#define USER_NOT_FOUND 202
#define WRONG_PASSWORD 203
#define NOT_ENOUGH_CASH 204
#define ERROR 205
#define CAN_NOT_LOGIN 206

#define CLIENTS_FOLDER_LENGTH 13
#define TOKENS_FOLDER_LENGTH 12

#define CLIENTS_FOLDER "data/clients/"
#define TOKENS_FOLDER "data/tokens/"

// Returns 0 if token is right and client is logged
// Fills login
int get_login(char *token, char *login);

// Get cash on client account by login
int get_money(char *login);

// Check if login and password exist
int authentication(char *login, char *passwd);

// Register new client
// Creates new file in CLIENTS_FOLDER
int register_client(char *login, char *passwd);

// Creates new file in TOKENS_FOLDER
// Function fills token
int login_client(char *login, char *token, int length);

// Send ammount of money to client with login <login> from client with token
// <token>
int send_to(char *token, char *login, int ammount);

// Delete client token from TOKENS_FOLDER
int delete_client_token(char *token);

// Delete client data from CLIENTS_FOLDER
int delete_client_data(char *login);

// Generates random string
void generate_token(char *token, int length);

void list_all(char *list_out);
