#include "files.h"

int withdraw_account_cash(char* login, int withdraw) {
	char* filename;
	char* buf[256];
	char* password[128];
	int length;
	int res;
	int cash;
	
	// Get cash
	bzero(password, 128);
	res = get_account_data(login, &cash, password);
	if (res != OK) {
		return res;
	}
	
	cash -= withdraw;
	
	// Create name of file
	length = strlen(CLIENTS_FOLDER) * sizeof(char) + strlen(login) * sizeof(char) + 1; // 1 for \0
	filename = (char*)malloc(length);
	bzero(filename, length);
	bcopy(CLIENTS_FOLDER, filename, strlen(CLIENTS_FOLDER) * sizeof(char));
	strcat(filename, login);
	
	// Put cash back to file
	bzero(buf, 256);
	sprintf(buf, "%d\n", cash);
	strcat(buf, password);
	res = write_to_file(filename, buf, strlen(buf));
	return res;
}
