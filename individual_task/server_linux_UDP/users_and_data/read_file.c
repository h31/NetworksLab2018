//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int read_file(char* filename, char* buf, int length)
{
    FILE* file;
    int res;

    // Open file for reading
    file = fopen(filename, "r");
    if (file == NULL) {
        return OPEN_FILE_ERROR;
    }

    // Read data from file to buf
    res = fread(buf, sizeof(char), length, file);

    // Check for errors
    if (res < length) {
        if (feof(file) == 0) {
            return READ_FILE_ERROR;
        }
    }

    // Close file
    res = fclose(file);
    if (res != 0) {
        return CLOSE_FILE_ERROR;
    }
    return OK;
}