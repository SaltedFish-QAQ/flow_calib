#ifndef _BSP_IIC_H_
#define _BSP_IIC_H_

#include <stdint.h>
#include "stm32f10x.h"

#define BSP_IIC_DELAY_MS    50

typedef enum
{
    iic_success = 0,
    iic_failed,
    iic_error_pull_push,
    iic_no_ack,
}bsp_iic_status_e;

typedef struct
{
    uint32_t        clk;
    GPIO_TypeDef    *port;
    uint16_t        pin;
}iic_gpio_obj_t;


typedef struct
{
    iic_gpio_obj_t scl;
    iic_gpio_obj_t sda;
}bsp_iic_obj_t;


bsp_iic_status_e bsp_iic_init(bsp_iic_obj_t *para);

bsp_iic_status_e bsp_iic_write_data(bsp_iic_obj_t *para, uint8_t device_address, uint8_t reg_address, uint8_t *data_pointer, uint8_t len);
bsp_iic_status_e bsp_iic_read_data(bsp_iic_obj_t *para, uint8_t device_address, uint8_t reg_address, uint8_t *data_pointer, uint8_t len);
bsp_iic_status_e bsp_iic_read(bsp_iic_obj_t *para, uint8_t device_address, uint8_t reg_address, uint8_t *data_pointer);

#endif
