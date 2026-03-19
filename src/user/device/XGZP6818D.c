#include "XGZP6818D.h"
#include "bsp_iic.h"
#include "stm32f10x_gpio.h"
#include <math.h>

#define XGZ6818D_ADDRESS        0x58

#define PMIN  -500.0
#define PMAX  500.0

bsp_iic_obj_t xgzp6818d_obj = 
{
    .scl = 
    {
        .clk = RCC_APB2Periph_GPIOB,
        .port = GPIOB,
        .pin = GPIO_Pin_6,
    },
    .sda = 
    {
        .clk = RCC_APB2Periph_GPIOB,
        .port = GPIOB,
        .pin = GPIO_Pin_7,
    },
};

uint8_t xgzp6818d_init(void)
{
    return (uint8_t)bsp_iic_init(&xgzp6818d_obj);
}

uint8_t xgzp6818d_read(float *pressure, float *temp)
{
    uint8_t data[5] = {0};
    uint8_t temp_calib_offset_code = 0, temp_calib_gain_exp = 0;
    int EOFF = 0;
    long int pressure_ad = 0, temperature_ad = 0;
    float pressure_value = 0, temperature_value = 0;
    bsp_iic_status_e stauts = iic_failed;

    stauts = bsp_iic_read_data(&xgzp6818d_obj, XGZ6818D_ADDRESS, 0x01, data, 1);

    if ((data[0] & 0x20) == 0)
    {
        return DATA_NOT_READY;
    }
    

    stauts = bsp_iic_read_data(&xgzp6818d_obj, XGZ6818D_ADDRESS, 0x04, data, 5);
    pressure_ad = (unsigned long)((((unsigned long)data[0]) << 16) |
                                    (((unsigned int)data[1]) << 8) |
                                            ((unsigned char)data[2]));

    temperature_ad = (unsigned long)(((unsigned int)data[3]) << 8) |
                                            ((unsigned char)data[4]);

    pressure_value = ((pressure_ad > 8388608) ?
                    (double)((pressure_ad - 16777216) / 2097152.0 * (PMAX - PMIN) ) :
                    (double)(pressure_ad / 2097152.0 * (PMAX - PMIN)));
    pressure_value = pressure_value + PMIN;

    stauts = bsp_iic_read_data(&xgzp6818d_obj, XGZ6818D_ADDRESS, 0x20, data, 1);
    temp_calib_offset_code = data[0];
    stauts = bsp_iic_read_data(&xgzp6818d_obj, XGZ6818D_ADDRESS, 0x21, data, 1);
    temp_calib_gain_exp = data[0];

    if (temp_calib_offset_code==0x0C)
        EOFF=4096;
    else if(temp_calib_offset_code==0x8C)
        EOFF=-4096;
    else if(temp_calib_offset_code==0x0D)
        EOFF=8192;
    else if(temp_calib_offset_code==0x8D)
        EOFF=-8192;
    else if(temp_calib_offset_code==0x0E)
        EOFF=16384;
    else if(temp_calib_offset_code==0x8E)
        EOFF=-16384;
    float  shift_N = pow(2,temp_calib_gain_exp/10);

    temperature_value = (((temperature_ad > 32768) ?
                        (temperature_ad - 65536) :
                        temperature_ad) - EOFF) / (shift_N) + 25.0;

    *pressure = pressure_value;
    *temp = temperature_value;

    return DATA_READ_SUCCESS;
}

