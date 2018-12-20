//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void clear_data(struct prime_numbers* data)
{
    data->range = 100000;
    data->current_range = 0;
    bzero(data->primes, sizeof(data->primes)/sizeof(data->primes[0]));
}