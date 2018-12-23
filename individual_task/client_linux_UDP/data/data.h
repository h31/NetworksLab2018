//
// Created by mrsandman on 13.12.18.
//

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <limits.h>

#define BUFSIZE 1024
#define SEND_SIZE 50

// Pack data to write to file
void pack_data(int data[], int size, char* buff);

// Calculates prime numbers in a range
int calculate_data(int data[], int current_range, int range);


