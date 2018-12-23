//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int read_file(char* filename, char* buf, int length)
{
    int file;
    int res;

    // Open file for reading
    if ((file = open(filename, O_RDONLY)) == -1) {
        return OPEN_FILE_ERROR;
    }

    // Read data from file to buf
    res = read(file, buf, length);

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