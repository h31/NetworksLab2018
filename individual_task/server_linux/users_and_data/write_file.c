//
// Created by mrsandman on 09.12.18.
//

#include "work.h"

int write_file(char* filename, char* buf, int length)
{
    FILE* file;
    int res;

    // Open file for reading
    file = fopen(filename, "w");
    if (file == NULL) {
        return OPEN_FILE_ERROR;
    }

    // Write data from buf to file
    res = fwrite(buf, sizeof(char), length, file);

    // Check for errors
    if (res < length) {
        if (feof(file) == 0) {
            return WRITE_FILE_ERROR;
        }
    }

    // Close file
    res = fclose(file);
    if (res != 0) {
        return CLOSE_FILE_ERROR;
    }
    return OK;
}