#include "fileManager.h"

/** Function to open file
 * param: char* name - name of file which you want to open
 * return: FILE* file - pointer to an object of type FILE*/
FILE *openFile(char *name) {

    FILE* file = fopen(name, "r+");

    if (file == NULL)
    {
        printf("Error opening file\n");
        exit(0);
    }
    return file;
}

/** Function to close file
 * param: char* name - name of file which you want to close
 * return: void */
void closeFile(FILE* file){
    fclose(file);
}

/** Function for reading a string from file with user's logins
 * param: char* name - name of file with logins
 *        int* position - pointer to a variable in which the user's position is read
 *        (1 - tester, 0 - developer)
 * return: char* login - read user's login */
char* readOneLogin(FILE *file, int *position) {

    char* login = (char*)calloc(MAX_BUFFER/2, sizeof(char));

    if(feof(file) != 0) {
        return NULL;
    }
    fscanf(file, "%s %d", login, position);

    return login;
}

/** Function for reading a repository information from special file
 * param: char* name - name of file with repositories
 * return: int i - number of records in the file */
int readRepositoryInfo(FILE *file) {

    if(feof(file) != 0) {
        return -1;
    }

    int i = 0;
    size_t len = 0;

    while(feof(file) == 0){
        fscanf(file, "%s %s %s %ld %s ",
               rep_data[i].dev_login,
               rep_data[i].test_login,
               rep_data[i].project,
               &rep_data[i].bugId,
               rep_data[i].bugStatus);
        getline(&rep_data[i].bugDescription, &len, file);
        rep_data[i].bugDescription[strcspn(rep_data[i].bugDescription, "\n")] = 0;

        if(rep_data[i].bugId > bugId){
            bugId = rep_data[i].bugId;
        }
        i++;
    }
    return i;
}

/** Function for writing editing repository information in the end of the session
 * param: char* name - name of file with repositories
 * return: void*/
void writeToRepositories(FILE *file){
    for(int i = 0; i < bugs_number; i++) {
        fprintf(file, "%s\n%s\n%s\n%ld\n%s\n%s", rep_data[i].dev_login, rep_data[i].test_login, rep_data[i].project,
                rep_data[i].bugId, rep_data[i].bugStatus, rep_data[i].bugDescription);
        if(i != bugs_number - 1){
            fprintf(file, "\n");
        }
    }
}