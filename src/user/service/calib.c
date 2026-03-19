#include "calib.h"
#include <stddef.h>

calib_status_e calib_data_set(calib_data_t *para, float adc, float val)
{
    if (para == NULL)
    {
        return calib_error;
    }
    para->adc = adc;
    para->val = val;

    return calib_success;
}

// Y0 = k * X0 + b
// Y1 : para[i+1].val, X1 : para[i+1].adc
// Y0 : para[i].val, X0 : para[i].adc
// k = (Y1 - Y0) / (X1 - X0)
// b = Y0 - k * X0
// Y = K * x + b
// Y = K * x + Y0- K * X0
// Y = K * (x- x0) + Y0
calib_status_e calib_data_calib(calib_data_t *para, uint8_t len, float adc, float *val)
{
    if (para == NULL)
    {
        return calib_error;
    }

    float k = 0;

    for (int i = 0; i < len - 1; i++)
    {
        if (para[i].val >= para[i + 1].val)
        {
            if (i == 0)
            {
                *val = adc;
                return calib_error_calib_data;
            }

            k = (para[i].val - para[i - 1].val) / (para[i].adc - para[i - 1].adc);
            *val = k * (adc - para[i - 1].adc) + para[i - 1].val;

            return calib_error_calib_data;
        }
        
        if (adc >= para[i].adc && adc <= para[i + 1].adc)
        {
            k = (para[i + 1].val - para[i].val) / (para[i + 1].adc - para[i].adc);
            *val = k * (adc - para[i].adc) + para[i].val;
            return calib_success;
        }

        if (i == len - 1)
        {
            k = (para[i].val - para[i - 1].val) / (para[i].adc - para[i - 1].adc);
            *val = k * (adc - para[i - 1].adc) + para[i - 1].val;
            return calib_over_data;
        }
    }

    return calib_error;
}

// data_after_correction = k * data + b
calib_status_e calib_data_correction(correction_data_t *para, float data, float *data_after_correction)
{
    if (para == NULL)
    {
        return calib_error;
    }
    *data_after_correction = para->k * data + para->b;

    return calib_success;
}
