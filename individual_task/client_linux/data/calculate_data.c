//
// Created by mrsandman on 13.12.18.
//

#include "data.h"

int calculate_data(int data[], int current_range, int range)
{
    int lb = current_range == 0 ? 2 : current_range * range + 1;
    int ub = (current_range + 1) * range;
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
            if (cur_size == SEND_SIZE) cur_size = 0;
            data[cur_size] = lb;
            cur_size++;
        }
        lb++;
    }


    // Sort all prime numbers
    for (int i = 0; i < SEND_SIZE - 1; i++) {
        for (int j = i; j < SEND_SIZE; j++) {
            if (data[i] > data[j]){
                int temp = data[j];
                data[j] = data[i];
                data[i] = temp;
            }
        }
    }

    return 0;
}