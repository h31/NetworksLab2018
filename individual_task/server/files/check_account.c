#include "files.h"

int check_account(char* login, char* password)
{
    char* filename;
    char* buf[256];
    char* str_token;
    int length;
    int res;

    // Create name of file
    length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
    filename = (char*)malloc(length);
    bzero(filename, length);
    bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
    strcat(filename, login);

    // Read data from file to buf
    bzero(buf, 256);
    res = read_from_file(filename, buf, 255);
    // Free memory
    free(filename);
    if (res != 0) {
        return USER_NOT_FOUND;
    }

    // Compare password
    str_token = strtok(buf, "\n"); // Read money
    str_token = strtok(NULL, "\n"); // Read password
    if (strcmp(password, str_token) != 0) {
        return WRONG_PASSWORD;
    }
    return OK;
}
