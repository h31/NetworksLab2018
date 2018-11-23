#include "files.h"

void init_data_folders()
{
    char* foldername;
    char* command;
    int length;

    // Create folder for clients
    // Allocate memory for clients folder name
    length = strlen(CLIENTS_FOLDER) * sizeof(char) + 1; // 1 for \0 symbol
    foldername = (char*)malloc(length);
    bzero(foldername, length - 1);
    bcopy(CLIENTS_FOLDER, foldername, length);
    // Allocate memory for command
    length += 9; // 9 is length of "mkdir -p "
    command = (char*)malloc(length);
    bzero(command, length);
    // Create command
    strcat(command, "mkdir -p ");
    strcat(command, foldername);
    // Execute command
    system(command);
    // Free memory
    free(foldername);
    free(command);

    // Create folder for session
    // Allocate memory for clients folder name
    length = strlen(SESSION_FOLDER) * sizeof(char) + 1; // 1 for \0 symbol
    foldername = (char*)malloc(length);
    bzero(foldername, length);
    bcopy(SESSION_FOLDER, foldername, length - 1);
    // Allocate memory for command
    length += 9; // 9 is length of "mkdir -p "
    command = (char*)malloc(length);
    bzero(command, length);
    // Create command
    strcat(command, "mkdir -p ");
    strcat(command, foldername);
    // Execute command
    system(command);
    // Free memory
    free(foldername);
    free(command);
}
