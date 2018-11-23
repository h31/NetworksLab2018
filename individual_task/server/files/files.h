#include "../logging/logging.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define error codes
#define OK 0
#define USER_ALREADY_EXISTS 201
#define USER_NOT_FOUND 202
#define WRONG_PASSWORD 203
#define NOT_ENOUGH_CASH 204
#define ERROR 205

// Define folders for data
#define CLIENTS_FOLDER "data/clients/"
#define SESSION_FOLDER "data/session/"

// Call this function on start. Creates folders for data
void init_data_folders();

// Basic functions
// Reads from <filename> into <buf> <length> of bytes
int read_from_file(char* filename, char* buf, int length);

// Write to <filename> bytes from <buf>
int write_to_file(char* filename, char* buf, int length);

// List all created accounts, length = max length of list_out
void list_all_accounts(char* list_out, int length);

// Account functions
// Create account with login and password
int create_account(char* login, char* password);

// Check if account with <login> exists and has a <password>
int check_account(char* login, char* password);

// Delete account by login
int delete_account(char* login);

// Get client cash. Function fills cash variable
int get_account_cash(char* login, int *cash);

// Add money to client account
int put_account_cash(char* login, int put);

// Withdraw moneys from account
int withdraw_account_cash(char* login, int withdraw);

// Session functions
// Create new session for client with <login>. Function fills token variable
int create_session(char* login, char* token, int token_length);

// Delete session file by token
int delete_session(char* token);

// Get client login by token. Function fills login variable
int get_session_client(char* token, char* login);

// Generates random string
void generate_token(char *token, int length);
