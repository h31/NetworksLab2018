#include "files.h"

int get_login(char* token, char* login) {
	FILE* f;
	char* filename;
	int res;
	
	// Get name of token file to read
	filename = (char*)malloc((TOKENS_FOLDER_LENGTH + strlen(token))*sizeof(char));
	sprintf(filename, "%s%s", TOKENS_FOLDER, token);
	
	// Check if file exists
    f = fopen(filename, "r");
    free(filename);
    if (f == NULL) {
    	return USER_NOT_FOUND;
    }
    
    // Read login
    fscanf(f, "%s", login);
    
    // Close file
    res = fclose(f);
    if (res != 0) {
    	return ERROR;
    }
	
    return FILES_OK;
}
