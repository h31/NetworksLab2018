#include "files.h"

int read_from_file(char* filename, char* buf, int length)
{
    FILE* file;
    int res;

    // Open file for reading
    file = fopen(filename, "r");
    if (file == NULL) {
        mlogf("ERROR on opening file with name=%s", filename);
        return ERROR;
    }

    // Read data from file to buf
    res = fread(buf, sizeof(char), length, file);

    // Check for errors
    if (res < length) {
        if (feof(file) == 0) {
            mlogf("ERROR on reading from file with name=%s", filename);
            return ERROR;
        }
    }

    // Close file
    res = fclose(file);
    if (res != 0) {
        mlogf("ERROR on closing file with name=%s", filename);
        return ERROR;
    }
    return OK;
}
