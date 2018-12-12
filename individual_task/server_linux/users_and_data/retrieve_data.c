//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void retrieve_data(struct prime_numbers* data, char* buff)
{
    char* primes;
    char* ranges;
    char* tmp;

    // Take primes, ranges and range from file
    primes = strtok(buff, ";");
    ranges = strtok(NULL, ";");
    tmp = strtok(NULL, "\n");

    // Fill server primes from file
    char* prime = strtok(primes, " ");
    data->primes[0] = atoi(prime);
    int i = 1;
    while (prime != NULL){
        prime = strtok(NULL, " ");
        if (prime == NULL) break;
        data->primes[i] = atoi(prime);
        i++;
    }

    // Fill server ranges from file
    char* range = strtok(ranges, " \0");
    data->ranges[0] = atoi(range);
    i = 1;
    while (range != NULL){
        range = strtok(NULL, " ");
        if (range == NULL) break;
        data->ranges[i] = atoi(range);
        i++;
    }

    data->range = atoi(tmp);
}