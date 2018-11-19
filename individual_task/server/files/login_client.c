#include "files.h"

int login_client(char* login, char* token, int length) {
    FILE* f;
	char* filename;
	int res;
	int iteration = 0;
	
	while(iteration < 10) {
	    // Generate token
	    generate_token(token, length);
	    
	    // Get name of file to create
	    filename = (char*)malloc(TOKENS_FOLDER_LENGTH + length);
	    sprintf(filename, "%s%s", TOKENS_FOLDER, token);
	    mlogf("generated token = %s", token);
	    
	    // Check if token already exists
        if (f = fopen(filename, "r")){
            fclose(f);
            free(filename);
            iteration++;
        } else {
            break;
        }
    }
    if(iteration >= 10) {
        return CAN_NOT_LOGIN;
    }
    
    system("mkdir -p data/tokens");
    
    // Open file for write
	f = fopen(filename, "w");
	free(filename);
	if(f == NULL) {
		mlog("ERROR. Can not create new token\n");
		return ERROR;
	}
	
	res = fprintf(f, "%s", login);
	if(res < strlen(login)) {
		mlogf("ERROR. %d/%d characters were written in token", res, strlen(login));
	}
	
	// Close file
	res = fclose(f);
	if(res != 0) {
		mlogf("ERROR. Can not close token file with error code = %d", res);
		return ERROR;
	}
	
	return FILES_OK;
}
