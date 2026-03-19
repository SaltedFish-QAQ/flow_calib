#include "sf_lib.h"
#include <stddef.h>
#include <string.h>

float average_float(float *float_array, uint32_t num)
{
    float sum_float = 0;

    for (int i = 0; i < num; i++)
    {
        sum_float = sum_float + *float_array++;
    }
    
    return (sum_float / num);
}

void bubble_sort_float(float *float_array, uint32_t num)
{
    for (int i = 0; i < num - 1; i++)
    {
        uint8_t is_stored = 0;

        for (int j = 0; j < num - 1 - i; j++)
        {
            if (float_array[j] > float_array[j + 1])
            {
                is_stored = 1;
                float temp = float_array[j];
                float_array[j] = float_array[j + 1];
                float_array[j + 1] = temp;
            }
        }
        if (is_stored == 0)
        {
            break;
        }
        
    }
    
}
