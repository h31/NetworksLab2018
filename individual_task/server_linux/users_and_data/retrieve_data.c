//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void retrieve_data(struct prime_numbers* data, char* buff)
{
    char* ptr;
    char* primes;
    char* range;
    char* current_range;

    // Take primes, ranges and range from file
    current_range = strtok(buff, ";");
    range = strtok(NULL, ";");
    primes = strtok(NULL, "\n");

    data->range = (int)strtol(range, &ptr, 10);
    data->current_range = (int)strtol(current_range, &ptr, 10);

    // Fill server primes from file
    char* prime = strtok(primes, " ");
    data->primes[0] = (int)strtol(prime, &ptr, 10);
    int i = 1;
    while (prime != NULL){
        prime = strtok(NULL, " ");
        if (prime == NULL) break;
        data->primes[i] = (int)strtol(prime, &ptr, 10);
        i++;
    }

}