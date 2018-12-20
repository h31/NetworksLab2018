//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int write_file(char* filename, char* buf, int length)
{
    int file;
    int res;

    // Open file for reading
    if ((file = open(filename, O_WRONLY | O_CREAT)) == -1) {
        return OPEN_FILE_ERROR;
    }

    // Read data from file to buf
    res = write(file, buf, length);

    // Check for errors
    if (res == -1) {
        return READ_FILE_ERROR;
    }

    // Close file
    if (close(file) == -1) {
        return CLOSE_FILE_ERROR;
    }
    return OK;
}