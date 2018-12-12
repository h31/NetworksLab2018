//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void clear_data(struct prime_numbers* data)
{
    bzero(data->ranges, sizeof(data->ranges) / sizeof(data->ranges[0]));
    bzero(data->primes, sizeof(data->primes) / sizeof(data->primes[0]));
}