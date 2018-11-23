#include "files.h"

int write_to_file(char* filename, char* buf, int length)
{
    FILE* file;
    int res;

    // Open file for reading
    file = fopen(filename, "w");
    if (file == NULL) {
        mlogf("ERROR on opening file with name=%s", filename);
        return ERROR;
    }

    // Read data from file to buf
    res = fwrite(buf, sizeof(char), length, file);

    // Check for errors
    if (res < length) {
        if (feof(file) == 0) {
            mlogf("ERROR on writing to file with name=%s", filename);
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
