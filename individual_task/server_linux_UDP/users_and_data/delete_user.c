//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int delete_user(char* login)
{
    char* filename;
    int length;
    int res;

    // Create name of file
    length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
    filename = (char*)malloc(length);
    bzero(filename, length);
    bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
    strcat(filename, login);

    res = remove(filename);
    if (res != 0) {
        return OTHER_ERROR;
    }
    return OK;
}