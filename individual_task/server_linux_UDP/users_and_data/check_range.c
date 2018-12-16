//
// Created by mrsandman on 13.12.18.
//

#include "work.h"

void check_range(struct prime_numbers* data, char* buff)
{
    char lowerbound[10] = {0};
    char upperbound[10] = {0};

    sprintf(lowerbound, "%d", data->current_range * 1000 + 1);
    sprintf(upperbound, "%d", (data->current_range + 1) * 1000);

    strcat(buff, "Your calculated range is [");
    strcat(buff, lowerbound);
    strcat(buff, ";");
    strcat(buff, upperbound);
    strcat(buff, "]");
}