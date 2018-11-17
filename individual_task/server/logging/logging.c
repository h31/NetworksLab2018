#include "logging.h"

void internal_log(char const * caller_func, char* msg) {
	FILE *f; // File to write in
	int res;
	char* output; // String to write in file
	time_t timer; // Variable to 
	char buffer[8]; // Buffer for milliseconds
    struct tm* tm_info; // Current time without milliseconds
    struct timeval tval; // Current milliseconds
	
	// Open file for append
	f = fopen("server.log", "a");
	if(f == NULL) {
		printf("ERROR. Can not open server.log\n");
	}
	
	// Get current time
    time(&timer);
    tm_info = localtime(&timer);
    gettimeofday(&tval, NULL);

	// Allocate memory for new string
	output = (char*) malloc((strlen(msg) + strlen(caller_func) + 31)*sizeof(char)); //31 = date(26) + 2 spaces + \n symbol + 2 brackets
	strftime(output, 20, "%Y-%m-%d %H:%M:%S", tm_info);
	sprintf(buffer, ".%06ld", (long int)tval.tv_usec);
	strcat(output, buffer);
	strcat(output, " [");
	strcat(output, caller_func);
	strcat(output, "] ");
	strcat(output, msg);
	strcat(output, "\n");
	
	// Write string to file
	res = fwrite(output, sizeof(char), strlen(output), f);
	if(res < strlen(msg)) {
		printf("ERROR. %d/%d characters were written in server.log", res, strlen(msg));
	}
	
	// Close file
	res = fclose(f);
	if(res != 0) {
		printf("ERROR. Can not close file server.log with error code = %d", res);
	}
}
