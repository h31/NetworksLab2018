#include "files.h"

int get_session_client(char* token, char* login)
{
    char* filename;
    char* buf[256];
    char* str_token;
    int length;
    int res;

    // Create name of file
    length = strlen(SESSION_FOLDER) * sizeof(char) + strlen(token) * sizeof(char) + 1; // 1 for \0
    filename = (char*)malloc(length);
    bzero(filename, length);
    bcopy(SESSION_FOLDER, filename, strlen(SESSION_FOLDER) * sizeof(char));
    strcat(filename, token);

    // Read data from file to buf
    bzero(buf, 256);
    res = read_from_file(filename, buf, 255);
    // Free memory
    free(filename);
    if (res != 0) {
        return res;
    }

    // Read money
    str_token = strtok(buf, "\n"); // Read money

    // Convert char array to int
    sscanf(str_token, "%s", login);

    return OK;
}
