#ifndef _CALIB_H_
#define _CALIB_H_

#include <stdint.h>

typedef enum
{
    calib_error = 0,
    calib_success,
    calib_over_data,
    calib_error_calib_data,
}calib_status_e;

typedef struct
{
    float adc;      // 测量得到的adc值
    float val;      // adc值对应的标定结果
}calib_data_t;

// y = k * x + b;
typedef struct
{
    float k;
    float b;
}correction_data_t;

calib_status_e calib_data_set(calib_data_t *para, float adc, float val);
calib_status_e calib_data_calib(calib_data_t *para, uint8_t len, float adc, float *val);
calib_status_e calib_data_correction(correction_data_t *para, float data, float *data_after_correction);

#endif
