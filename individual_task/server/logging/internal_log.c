#include "logging.h"

void internal_log(char const* caller_func, char* msg)
{
    int res;
    char* output; // String to write in file
    time_t timer; // Variable to
    char buffer[8]; // Buffer for milliseconds
    struct tm* tm_info; // Current time without milliseconds
    struct timeval tval; // Current milliseconds

    // Get current time
    time(&timer);
    tm_info = localtime(&timer);
    gettimeofday(&tval, NULL);

    // Allocate memory for new string
    // 32 = date(26) + 2 spaces + \n symbol + 2 brackets + null terminated symbol
    output = (char*)malloc((strlen(msg) + strlen(caller_func) + 32) * sizeof(char));
    strftime(output, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    sprintf(buffer, ".%06ld", (long int)tval.tv_usec);
    strcat(output, buffer);
    strcat(output, " [");
    strcat(output, caller_func);
    strcat(output, "] ");
    strcat(output, msg);
    strcat(output, "\n");

    // Write string to console
    if (console_log) {
        printf("%s", output);
    }

    // Write string to file
    flockfile(file_to_log);
    res = fwrite(output, sizeof(char), strlen(output), file_to_log);
    funlockfile(file_to_log);
    if (res < strlen(output)) {
        printf("ERROR. %d/%d characters were written in server.log", res,
            strlen(output));
        // fclose(file_to_log);
    }

    // Clear memory
    free(output);
}
