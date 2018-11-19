#include "files.h"

int authentication(char* login, char* passwd) {
	FILE* f;
	char* filename;
	int res;
	char _passwd[256];
	
	// Get name of file to create
	filename = (char*)malloc((CLIENTS_FOLDER_LENGTH + strlen(login))*sizeof(char));
	sprintf(filename, "%s%s", CLIENTS_FOLDER, login);
	
	// Open file for reading
    f = fopen(filename, "r");
    if (f == NULL) {
        free(filename);
        return USER_NOT_FOUND;
    }
    free(filename);
    
    // Check passwords
    fscanf(f, "%d", &res); // skip money
    fscanf(f, "%s", &_passwd);
    if(strcmp(_passwd, passwd) != 0) {
    	fclose(f);
    	return WRONG_PASSWORD;
    }
    
    // Close file
	res = fclose(f);
	if(res != 0) {
		printf("ERROR. Can not close client file with error code = %d", res);
		return ERROR;
	}
	
    return FILES_OK;
}
