#include "logging.h"

void log_init(char* filename)
{
    file_to_log = fopen(filename, "a");
    if (file_to_log == NULL) {
        printf("ERROR. Can not open %s\n", filename);
    }
}
