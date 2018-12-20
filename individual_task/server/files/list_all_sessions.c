#include "files.h"

void list_all_sessions(char* list_out, int length)
{
    char buf[100];
    char buf2[100];
    int list_pointer = 0;
    int written = 0;
    DIR* d;
    struct dirent* dir;
    int filename_length;
    char* filename;

    // Set list_out to zero
    bzero(list_out, length);
    d = opendir(SESSION_FOLDER);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            // Set buf to zero
            bzero(buf, 100);
            // Write folder name to buf
            written = sprintf(buf, "%s", dir->d_name);
            // Read data from folder, if folder != '.' or '..'
            if (strcmp(buf, ".") != 0 && strcmp(buf, "..") != 0) {
            	// Create name of file
    			filename_length = strlen(SESSION_FOLDER) * sizeof(char) + strlen(buf) * sizeof(char) + 1; // 1 for \0
    			filename = (char*)malloc(filename_length);
    			bzero(filename, filename_length);
    			bcopy(SESSION_FOLDER, filename, strlen(SESSION_FOLDER) * sizeof(char));
    			strcat(filename, buf);
            	// Set buf2 to zero
            	bzero(buf2, 100);
            	// Read from file to buf
            	read_from_file(filename, buf2, 100);
                bcopy(buf2, &list_out[list_pointer], strlen(buf2));
                list_pointer += strlen(buf2);
                //add token
                bzero(buf2, 100);
                sprintf(buf2, " - %s", buf);
                bcopy(buf2, &list_out[list_pointer], strlen(buf2));
                list_pointer += strlen(buf2);
            
                
                if (list_pointer > length) {
                    closedir(d);
                    return;
                }
            }
        }
    }
    if(list_pointer == 0) {
    	bzero(buf, 100);
    	sprintf(buf, "empty");
    	bcopy(buf, &list_out[0], 6);
    }
    closedir(d);
}
