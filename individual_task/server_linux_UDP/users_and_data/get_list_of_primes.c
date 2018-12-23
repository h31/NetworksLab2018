//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

int get_list_of_primes(struct prime_numbers* data, char* primes, char* count)
{
    char* ptr;
    int n = (int)strtol(count, &ptr, 10);
    int tmp[n];

    // Check if size of count correct
    if (n <= 0 || n > PRIMESSIZE)
    {
        return INCORRECT_COUNT;
    }

    bzero(tmp, n);

    // Fill temporary array with prime numbers
    for (int i = PRIMESSIZE - 1; i >= PRIMESSIZE - n - 1; i--) {
        tmp[PRIMESSIZE - 1 - i] = data->primes[i];
    }

    // Convert array of prime numbers to string
    int index = 0;
    for (int i = 0; i < n; i++) {
        index += sprintf(&primes[index], "%d ", tmp[i]);
    }

    return OK;
}