//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int create_session(char* login, char* token, int token_length)
{
    char* filename;
    char* buf;
    int length;
    int res;

    // Generate token
    generate_token(token, token_length);

    // Create name of file (name = login)
    length = strlen(SESSION_FOLDER) * sizeof(char) + token_length * sizeof(char) + 1; // 1 for \0
    filename = (char*)malloc(length);
    bzero(filename, length);
    bcopy(SESSION_FOLDER, filename, strlen(SESSION_FOLDER) * sizeof(char));
    strcat(filename, token);

    // Create buf for writing
    length = strlen(login) * sizeof(char);
    buf = (char*)malloc(length);
    bzero(buf, length);

    // Put data (login) into buf
    bcopy(login, buf, strlen(login) * sizeof(char));

    // Write to file
    res = write_file(filename, buf, length);

    // Free memory
    free(filename);
    free(buf);

    return res;
}