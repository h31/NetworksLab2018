#include "files.h"

int register_client(char* login, char* passwd) {
	FILE* f;
	char* filename;
	int res;
	
	// Get name of file to create
	filename = (char*)malloc((CLIENTS_FOLDER_LENGTH + strlen(login))*sizeof(char));
	sprintf(filename, "%s%s", CLIENTS_FOLDER, login);
	
	// Check if file already exists
    if (f = fopen(filename, "r")){
        fclose(f);
        free(filename);
        return USER_ALREADY_EXISTS;
    }
    
    system("mkdir -p data/clients");
    
    // Open file for write
	f = fopen(filename, "w");
	free(filename);
	if(f == NULL) {
		mlog("ERROR. Can not create new client");
		return ERROR;
	}
	
	// Write money to file
	res = fprintf(f, "1000\n");
	if(res < 1) {
		mlogf("ERROR. %d/%d characters were written in client file", res, 1);
		return ERROR;
	}
	
	// Write password next line
	res = fprintf(f, "%s", passwd);
	if(res < strlen(passwd)) {
		mlogf("ERROR. %d/%d characters were written in client file", res, strlen(passwd));
		return ERROR;
	}
	
	// Close file
	res = fclose(f);
	if(res != 0) {
		mlogf("ERROR. Can not close client file with error code = %d", res);
		return ERROR;
	}
	
	return FILES_OK;
}
