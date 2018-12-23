#ifndef BUGTRACKER_FILEMANAGER_H
#define BUGTRACKER_FILEMANAGER_H

#include "../includes.h"

FILE *openFile(char *name);
char* readOneLogin(FILE *file, int *position);
int readRepositoryInfo(FILE *file);
void writeToRepositories(FILE *file);
void closeFile(FILE* file);

#endif //BUGTRACKER_FILEMANAGER_H

