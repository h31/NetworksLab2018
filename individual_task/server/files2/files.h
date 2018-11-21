#include "../logging/logging.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define error codes
#define USER_ALREADY_EXISTS 201
#define USER_NOT_FOUND 202
#define WRONG_PASSWORD 203
#define NOT_ENOUGH_CASH 204
#define ERROR 205

// Define folders for data
#define CLIENTS_FOLDER "data/clients/"
#define SESSION_FOLDER "data/session/"

// Call this function on start. Creates folders for data
int init_data_folders()

// Basic functions
// Reads from <file> into <buf> <length> of bytes
int read_from_file(FILE* file, char* buf, int length);

// Write to <file> bytes from <buf>
int write_to_file(FILE* file, char* buf);

// Function checks if file <filename> is created
int is_file_created(FILE* file, char* filename);

// Account functions
// Create account with login and password
int create_account(char* login, char* password);

// Check if account with <login> exists and has a <password>
int check_account(char* login, char* password);

// Delete account by login
int delete_account(char* login);

// Session functions
// Create new session for client with <login> and <password>. Function fills
// token variable
int create_session(char* login, char* password, char* token);

// Delete session file by token
int delete_session(char* token);
