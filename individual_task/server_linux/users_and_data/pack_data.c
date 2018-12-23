//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void pack_data(struct prime_numbers* data, char* buff)
{
    char * primes = (char *) malloc(PRIMESSIZE);
    char range[10] = {0};
    char current_range[10] = {0};

    sprintf(current_range, "%d", data->current_range);
    strcat(current_range, ";");

    sprintf(range, "%d", data->range);
    strcat(range, ";");

    // Convert server primes to string
    int index = 0;
    for (int i = 0; i < PRIMESSIZE; i++) {
        if (i == PRIMESSIZE - 1) index += sprintf(&primes[index], "%d", data->primes[i]);
        else index += sprintf(&primes[index], "%d ", data->primes[i]);
    }

    // Join all strings to buffer
    strcat(buff, current_range);
    strcat(buff, range);
    strcat(buff, primes);

    free(primes);

}