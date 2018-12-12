//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

int calculate_data(struct prime_numbers* data, char* lowerbound, char* upperbound)
{
    char* ptr;
    int lb = strtol(lowerbound,&ptr,10);
    int ub = strtol(upperbound,&ptr,10);
    int flag;
    int cur_size = -1, new_size = 0;

    // Check if bounds are positive
    if (lb < 0 || ub < 0)
    {
        return NEGATIVE_BOUND;
    }

    // Check if bounds are correct
    if (lb > ub)
    {
        return INCORRECT_BOUNDS;
    }

    // Check if bounds are multiple to range
    if (lb % data->range != 0 || ub % data->range != 0)
    {
        return BOUND_IS_NOT_MULTIPLE;
    }

    // Check if range is already used
    for (int i = lb/data->range; i < ub/data->range; ++i) {
        if (data->ranges[i] == 1)
        {
            return RANGE_IS_ALREADY_USED;
        }
        data->ranges[i] = 1;
    }

    // Find current count of prime numbers
    for(int i = 0; i < sizeof(data->primes)/sizeof(data->primes[0]); i++){
        if (data->primes[i] == 0) break;
        cur_size++;
    }

    if (lb == 0) lb = 1;

    // Calculate prime numbers
    while (lb < ub)
    {
        flag = 0;
        for(int i = 2; i <= lb/2; i++)
        {
            if(lb % i == 0)
            {
                flag = 1;
                break;
            }
        }
        if (flag == 0)
        {
            data->primes[cur_size] = lb;
            cur_size++;
        }
        ++lb;
    }

    new_size = cur_size;

    // Sorting prime numbers (ascending)
    for (int i = 0; i < new_size - 1; i++) {
        for (int j = 0; j < new_size-i-1; j++) {
            if(data->primes[j] > data->primes[j+1])
            {
                int temp = data->primes[j];
                data->primes[j]=data->primes[j+1];
                data->primes[j+1]=temp;
            }
        }
    }

    return OK;
}