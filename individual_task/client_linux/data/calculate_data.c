//
// Created by mrsandman on 13.12.18.
//

#include "data.h"

int calculate_data(int data[], int range)
{
    char* ptr;
    int lb = range == 0 ? 2 : range * 1000 + 1;
    int ub = (range + 1) * 1000;
    int flag;
    int cur_size = 0;

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
            data[cur_size] = lb;
            cur_size++;
        }
        lb++;
    }

    return 0;
}