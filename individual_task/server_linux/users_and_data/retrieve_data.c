//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void retrieve_data(struct prime_numbers* data, char* buff)
{
    char* ptr;
    char* primes;
    char* ranges;
    char* tmp;

    // Take primes, ranges and range from file
    primes = strtok(buff, ";");
    ranges = strtok(NULL, ";");
    tmp = strtok(NULL, "\n");

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

    // Fill server ranges from file
    char* range = strtok(ranges, " \0");
    data->ranges[0] = (int)strtol(range, &ptr, 10);
    i = 1;
    while (range != NULL){
        range = strtok(NULL, " ");
        if (range == NULL) break;
        data->ranges[i] = (int)strtol(range, &ptr, 10);
        i++;
    }

    data->range = (int)strtol(tmp, &ptr, 10);
}