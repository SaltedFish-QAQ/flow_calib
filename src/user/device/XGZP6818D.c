#include "XGZP6818D.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "bsp_iic.h"
#include "bsp_delay.h"
#include <math.h>

#define XGZ6818D_ADDRESS        0x58

#define PMIN  -500.0
#define PMAX  500.0

static void xgzp6818d_iic_init(void);
static void xgzp6818d_iic_scl_set(uint8_t para);
static void xgzp6818d_iic_sda_set(uint8_t para);
static uint8_t xgzp6818d_iic_sda_get(void);
static void xgzp6818d_iic_delay_func(uint32_t para);

bsp_iic_obj_t xgzp6818d_obj = 
{
    .iic_init = xgzp6818d_iic_init,
    .iic_scl_set = xgzp6818d_iic_scl_set,
    .iic_sda_set = xgzp6818d_iic_sda_set,
    .iic_sda_get = xgzp6818d_iic_sda_get,
    .iic_delay_func = xgzp6818d_iic_delay_func,
    .iic_delay_ms = BSP_IIC_DELAY_MS,
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

static void xgzp6818d_iic_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_StructInit(&GPIO_InitStructure);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // 初始化当前对象
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure); // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure); // 初始化GPIO

    GPIO_SetBits(GPIOB, GPIO_Pin_7);
    GPIO_SetBits(GPIOB, GPIO_Pin_6);
}

static void xgzp6818d_iic_scl_set(uint8_t para)
{
    if (para == 0)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_6);
    }
    else if (para == 1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_6);
    }
    
}

static void xgzp6818d_iic_sda_set(uint8_t para)
{
    if (para == 0)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_7);
    }
    else if (para == 1)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_7);
    }
}

static uint8_t xgzp6818d_iic_sda_get(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
}

static void xgzp6818d_iic_delay_func(uint32_t para)
{
    bsp_delay_ms(para);
}
