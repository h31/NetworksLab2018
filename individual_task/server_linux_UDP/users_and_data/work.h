//
// Created by mrsandman on 09.12.18.
//

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Define error codes
#define OK 0
#define USER_ALREADY_EXISTS 101
#define USER_NOT_FOUND 102
#define OPEN_FILE_ERROR 103
#define READ_FILE_ERROR 104
#define WRITE_FILE_ERROR 105
#define CLOSE_FILE_ERROR 106
#define BOUND_IS_NOT_MULTIPLE 107
#define RANGE_IS_ALREADY_USED 108
#define NEGATIVE_BOUND 109
#define INCORRECT_BOUNDS 110
#define INCORRECT_COUNT 111
#define OTHER_ERROR 112

// Define folders for data
#define CLIENTS_FOLDER "data/clients/"
#define SESSION_FOLDER "data/session/"

struct prime_numbers{
    int current_range;
    int range;
    int primes[10000];
};

// Call this function on start. Creates folders for data
void create_data_storage();

// Basic functions
// Reads from <filename> into <buf> <length> of bytes
int read_file(char* filename, char* buf, int length);

// Write to <filename> bytes from <buf>
int write_file(char* filename, char* buf, int length);

// Account functions
// Create account with login
int create_user(char* login);

// Check if account with <login> exists
int check_user(char* login);

// Delete account by login
int delete_user(char* login);

// Session functions
// Create new session for client with <login>. Function fills token variable
int create_session(char* login, char* token, int token_length);

// Delete session file by token
int delete_session(char* token);

// Get client login by token. Function fills login variable
int get_session_client(char* token, char* login);

// Generates random string
void generate_token(char* token, int length);

// Clears server data
void clear_data(struct prime_numbers* data);

// Retrieve data from file
void retrieve_data(struct prime_numbers* data, char* buff);

// Pack data to write to file
void pack_data(struct prime_numbers* data, char* buff);

// Calculates prime numbers in a range
void check_range(struct prime_numbers* data, char* buff);

//Find maximum prime number from server data
int find_maxprime(struct prime_numbers* data);

//Find last N prime numbers
int get_list_of_primes(struct prime_numbers* data, char* primes, char* count);