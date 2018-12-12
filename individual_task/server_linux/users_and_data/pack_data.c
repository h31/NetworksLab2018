//
// Created by mrsandman on 12.12.18.
//

#include "work.h"

void pack_data(struct prime_numbers* data, char* buff)
{
    char primes[sizeof(data->primes)/sizeof(data->primes[0])] = {0};
    char ranges[sizeof(data->ranges)/sizeof(data->ranges[0])] = {0};
    char tmp[10] = {0};

    int cur_primes = 0;
    int cur_ranges = 0;

    // Find current count of prime numbers
    for (int i = 0; i < sizeof(data->primes)/sizeof(data->primes[0]); i++) {
        if (data->primes[i] == 0) break;
        cur_primes++;
    }

    // Find current count of ranges numbers
    for (int i = 0; i < sizeof(data->ranges)/sizeof(data->ranges[0]); i++) {
        if (data->ranges[i] == 0) break;
        cur_ranges++;
    }

    // Convert server primes to string
    int index = 0;
    for (int i = 0; i < cur_primes; i++) {
        if (i == cur_primes - 1) index += sprintf(&primes[index], "%d", data->primes[i]);
        else index += sprintf(&primes[index], "%d ", data->primes[i]);
    }
    strcat(primes, ";");

    // Convert server ranges to string
    index = 0;
    for (int i = 0; i < cur_ranges; i++) {
        if (i == cur_ranges - 1) index += sprintf(&ranges[index], "%d", data->ranges[i]);
        else index += sprintf(&ranges[index], "%d ", data->ranges[i]);
    }
    strcat(ranges, ";");

    sprintf(tmp, "%d", data->range);

    // Join all strings to buffer
    strcat(ranges, tmp);
    strcat(primes, ranges);
    strcat(buff, primes);

}