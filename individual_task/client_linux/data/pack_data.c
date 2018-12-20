//
// Created by mrsandman on 13.12.18.
//

#include "data.h"

void pack_data(int data[], int size,  char* buff)
{
    char * primes = (char *) malloc(50000);

    int cur_primes = 0;

    // Find current count of prime numbers
    for (int i = 0; i < size; i++) {
        if (data[i] == 0) break;
        cur_primes++;
    }

    // Convert server primes to string
    int index = 0;
    for (int i = 0; i < cur_primes; i++) {
        if (i == cur_primes - 1) index += sprintf(&primes[index], "%d", data[i]);
        else index += sprintf(&primes[index], "%d ", data[i]);
    }

    strcat(buff, primes);

    free(primes);
}