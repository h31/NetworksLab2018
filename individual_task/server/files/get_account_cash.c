#include "files.h"

int get_account_cash(char* login, int *cash) {
	char* filename;
	char* buf[256];
	char* str_token;
	int length;
	int res;
	
	// Create name of file
	length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
	filename = (char*)malloc(length);
	bzero(filename, length);
	bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
	strcat(filename, login);
	
	// Read data from file to buf
	bzero(buf, 256);
	res = read_from_file(filename, buf, 255);
	// Free memory
	free(filename);
	if(res != 0) {
		return res;
	}
	
	// Read money
	str_token = strtok(buf, "\n"); // Read money
	
	// Convert char array to int
	sscanf(str_token, "%d", &*cash);
	
	return OK;
}
