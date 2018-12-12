//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

int get_list_of_primes(struct prime_numbers* data, char* primes, char* count)
{
    char* ptr;
    int n = strtol(count,&ptr,10);
    int tmp[n];
    int cur_size = 0;

    // Check if size of count correct
    if (n <= 0)
    {
        return INCORRECT_COUNT;
    }

    // Find current count of prime numbers
    for(int i = 0; i < sizeof(data->primes)/sizeof(data->primes[0]); i++){
        if (data->primes[i] == 0) break;
        cur_size++;
    }

    bzero(tmp, n);

    // Fill temporary array with prime numbers
    for (int i = cur_size - 1; i >= cur_size - n - 1; i--) {
        tmp[cur_size - 1 - i] = data->primes[i];
    }

    // Convert array of prime numbers to string
    int index = 0;
    for (int i = 0; i < n; i++) {
        index += sprintf(&primes[index], "%d ", tmp[i]);
    }

    return OK;
}