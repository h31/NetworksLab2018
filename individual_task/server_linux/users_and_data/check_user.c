//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int check_user(char* login)
{
    char* filename;
    char* buf[256];
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
    res = read_file(filename, buf, 255);

    free(filename);
    if (res != 0) {
        return USER_NOT_FOUND;
    }

    return OK;
}