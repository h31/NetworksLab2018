//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void clear_data(struct prime_numbers* data)
{
    data->range = 1000;
    data->current_range = 0;
    for (int i = 0; i < (int) (sizeof(data->primes)/sizeof(data->primes[0])); ++i) {
        data->primes[i] = 0;
    }
}