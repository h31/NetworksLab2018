//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int create_user(char* login)
{
    char* filename;
    char* buf;
    int length;
    int res;

    // Create name of file (name = login)
    length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
    filename = (char*)malloc(length);
    bzero(filename, length);
    bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
    strcat(filename, login);

    // Create buf for writing
    buf = (char*)malloc(length);
    bzero(buf, length);

    // Write to file
    res = write_file(filename, buf, length);

    // Free memory
    free(filename);
    free(buf);

    return res;
}