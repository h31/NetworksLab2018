#include "files.h"

void list_all(char *list_out) {
  char list[1024];
  char buf[50];
  int list_pointer = 0;
  int written = 0;
  DIR *d;
  struct dirent *dir;

  bzero(list, 1024);
  d = opendir(CLIENTS_FOLDER);
  if (d) {
    while ((dir = readdir(d)) != NULL) {
      mlogf("Try to write with pointer = %d", list_pointer);

      bzero(buf, 50);
      written = sprintf(buf, "%s\n", dir->d_name);
      mlogf("written = %d", written);

      if (strcmp(buf, ".\n") != 0 && strcmp(buf, "..\n") != 0) {
        bcopy(buf, &list[list_pointer], written);
        list_pointer += written;
      }
    }
    closedir(d);
  }
  int i;
  for (i = 0; i < 1024; i++) {
    list_out[i] = list[i];
  }
}
