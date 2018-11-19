#include "files.h"

int delete_client_token(char* token) {
	FILE* f;
	char* filename;
	int res;
	
	filename = (char*)malloc((TOKENS_FOLDER_LENGTH + strlen(token))*sizeof(char));
	sprintf(filename, "%s%s", TOKENS_FOLDER, token);
	
	res = remove(filename);
	return res;
}


