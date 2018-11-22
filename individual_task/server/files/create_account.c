#include "files.h"

int create_account(char* login, char* password) {
	char* filename;
	char* buf;
	int length;
	int res;
	
	// Create name of file (name = login)
	length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
	filename = (char*)malloc(length);
	bzero(filename, length);
	bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
	strcat(filename, login);
	
	// Create buf for writing
	length = 4 * sizeof(char); // Size of '1000' - start account cash
	length += strlen(password) * sizeof(char) + 1; // 1 = \n
	buf = (char*)malloc(length);
	bzero(buf, length);
	
	// Put data (money and password) into buf
	bcopy("1000\n", buf, 5*sizeof(char));
	bcopy(password, &buf[5], strlen(password) * sizeof(char));

	// Write to file
	res = write_to_file(filename, buf, length);
	
	// Free memory
	free(filename);
	free(buf);
	
	return res;
}
