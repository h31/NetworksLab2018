//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

int find_maxprime(struct prime_numbers* data)
{

    int max = data->primes[0];

    // Find current count of prime numbers
    for (int i = 0; i < PRIMESSIZE; i++) {
        if (max < data->primes[i])
            max = data->primes[i];
    }

    return max;
}