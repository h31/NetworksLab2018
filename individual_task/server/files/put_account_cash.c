#include "files.h"

int put_account_cash(char* login, int put) {
	char* filename;
	char* buf[256];
	char* str_token;
	int length;
	int res;
	int cash;
	
	// Get cash
	res = get_account_cash(login, &cash);
	if (res != OK) {
		return res;
	}
	
	cash += put;
	
	// Create name of file
	length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
	filename = (char*)malloc(length);
	bzero(filename, length);
	bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
	strcat(filename, login);
	
	// Put cash back to file
	bzero(buf, 256);
	sprintf(buf, "%d", cash);
	res = write_to_file(filename, buf, strlen(buf));
	return res;
}
