#include "files.h"

int delete_client_data(char* login) {
	FILE* f;
	char* filename;
	int res;
	
	filename = (char*)malloc((CLIENTS_FOLDER_LENGTH + strlen(login))*sizeof(char));
	sprintf(filename, "%s%s", CLIENTS_FOLDER, login);
	
	res = remove(filename);
	return res;
}
