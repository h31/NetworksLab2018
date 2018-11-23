#include "files.h"

void list_all_accounts(char* list_out, int length) {
	char buf[50];
	int list_pointer = 0;
	int written = 0;
	DIR* d;
	struct dirent* dir;

	// Set list_out to zero
	bzero(list_out, length);
	d = opendir(CLIENTS_FOLDER);
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			// Set buf to zero
			bzero(buf, 50);
			// Write folder name to buf
			written = sprintf(buf, "%s\n", dir->d_name);
			// Add buf to list, if folder != '.' or '..'
			if (strcmp(buf, ".\n") != 0 && strcmp(buf, "..\n") != 0) {
				bcopy(buf, &list_out[list_pointer], written);
				list_pointer += written;
				if(list_pointer > length) {
					closedir(d);
					return;
				}
			}
		}
	}
	closedir(d);
}
