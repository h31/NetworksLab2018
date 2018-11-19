#include "files.h"

int get_money(char* login) {
	FILE* f;
	char* filename;
	int res;
	int money;
	
	// Get name of token file to read
	filename = (char*)malloc((CLIENTS_FOLDER_LENGTH + strlen(login))*sizeof(char));
	sprintf(filename, "%s%s", CLIENTS_FOLDER, login);
	
	//mlog("get file");
	// Check if file exists
    f = fopen(filename, "r");
    free(filename);
    if (f == NULL) {
    	return -1;
    }

    // Read login
    fscanf(f, "%d", &money);
    
    // Close file
    res = fclose(f);
    if (res != 0) {
    	return -1;
    }
	
    return money;
}
