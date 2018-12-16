//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int delete_session(char* token)
{
    char* filename;
    int length;
    int res;

    // Create name of file
    length = strlen(SESSION_FOLDER) * sizeof(char) + strlen(token) * sizeof(char) + 1; // 1 for \0
    filename = (char*)malloc(length);
    bzero(filename, length);
    bcopy(SESSION_FOLDER, filename, strlen(SESSION_FOLDER) * sizeof(char));
    strcat(filename, token);

    res = remove(filename);
    if (res != 0) {
        return OTHER_ERROR;
    }

    return OK;
}