//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

int find_maxprime(struct prime_numbers* data)
{

    int cur_size = 0;

    // Find current count of prime numbers
    for (int i = 0; i < (int)(sizeof(data->primes)/sizeof(data->primes[0])); i++) {
        if (data->primes[i] == 0) break;
        cur_size++;
    }

    return data->primes[cur_size-1];
}