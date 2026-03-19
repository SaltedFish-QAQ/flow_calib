#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "bsp_iic.h"
#include "bsp_delay.h"
#include <stddef.h>

static void iic_sda_high(bsp_iic_obj_t *para);
static void iic_sda_low(bsp_iic_obj_t *para);

static void iic_scl_high(bsp_iic_obj_t *para);
static void iic_scl_low(bsp_iic_obj_t *para);

static uint8_t iic_read_sda(bsp_iic_obj_t *para);

static bsp_iic_status_e _iic_start(bsp_iic_obj_t *para);
static void _iic_stop(bsp_iic_obj_t *para);
static void _iic_ack(bsp_iic_obj_t *para);
static void _iic_nack(bsp_iic_obj_t *para);
static bsp_iic_status_e _iic_wait_ack(bsp_iic_obj_t *para);
static void _iic_send_byte(bsp_iic_obj_t *para, uint8_t data);
static uint8_t _iic_read_byte(bsp_iic_obj_t *para);

bsp_iic_status_e bsp_iic_init(bsp_iic_obj_t *para)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    if (para->sda.clk == NULL || para->scl.clk == NULL)
    {
        return iic_failed;
    }

    GPIO_StructInit(&GPIO_InitStructure);
    RCC_APB2PeriphClockCmd(para->sda.clk, ENABLE);
    RCC_APB2PeriphClockCmd(para->scl.clk, ENABLE);

    // 初始化当前对象
    GPIO_InitStructure.GPIO_Pin = para->sda.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz
    GPIO_Init(para->sda.port, &GPIO_InitStructure); // 初始化GPIO

    GPIO_InitStructure.GPIO_Pin = para->scl.pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 50MHz
    GPIO_Init(para->scl.port, &GPIO_InitStructure); // 初始化GPIO

    GPIO_SetBits(para->scl.port, para->scl.pin);
    GPIO_SetBits(para->sda.port, para->sda.pin);

    return iic_success;
}

bsp_iic_status_e bsp_iic_write_data(bsp_iic_obj_t *para, uint8_t device_address, uint8_t reg_address, uint8_t *data_pointer, uint8_t len)
{
    if (para->sda.clk == NULL || para->scl.clk == NULL)
    {
        return iic_failed;
    }

    bsp_iic_status_e status = iic_failed;

    // 启动iic总线
    status = _iic_start(para);
    if (status != iic_success)
    {
        return status;
    }
    
    // 发送从设备地址
    _iic_send_byte(para, ((device_address << 1) | 0x00));
    status = _iic_wait_ack(para);
    if (status != iic_success)
    {
        _iic_stop(para);
        return status;
    }
        
    // 发送寄存器地址
    _iic_send_byte(para, reg_address);
    // _iic_wait_ack(para);
    status = _iic_wait_ack(para);
    if (status != iic_success)
    {
        _iic_stop(para);
        return status;
    }

    for (int i = 0; i < len; i++)
    {
        _iic_send_byte(para, data_pointer[i]);
        status = _iic_wait_ack(para);
        if (status != iic_success)
        {
            _iic_stop(para);
            return status;
        }
    }

    _iic_stop(para);
    return iic_success;
}

bsp_iic_status_e bsp_iic_read_data(bsp_iic_obj_t *para, uint8_t device_address, uint8_t reg_address, uint8_t *data_pointer, uint8_t len)
{
    if (para->sda.clk == NULL || para->scl.clk == NULL)
    {
        return iic_failed;
    }

    bsp_iic_status_e status = iic_failed;

    // 发送起始信号
    status = _iic_start(para);
    if (status != iic_success)
    {
        return status;
    }

    // 发送从设备地址
    _iic_send_byte(para, ((device_address << 1) | 0x00));
    status = _iic_wait_ack(para);
    if (status != iic_success)
    {
        _iic_stop(para);
        return status;
    }

    // 发送寄存器地址
    _iic_send_byte(para, reg_address);
    status = _iic_wait_ack(para);
    if (status != iic_success)
    {
        _iic_stop(para);
        return status;
    }

    // 发送停止信号
    _iic_stop(para);

    // 开始读模式
    status = _iic_start(para);
    if (status != iic_success)
    {
        return status;
    }

    _iic_send_byte(para, ((device_address << 1) | 0x01));
    status = _iic_wait_ack(para);
    if (status != iic_success)
    {
        _iic_stop(para);
        return status;
    }

    while (len)
    {
        // 读取一个字节的数据到缓冲区
        *data_pointer = _iic_read_byte(para);
        if (len == 1)
        {
            // 如果是最后一个字节，发送非应答信号
            _iic_nack(para);
        }
        else
        {
            // 如果不是最后一个字节，发送应答信号
            _iic_ack(para);
        }
        // 移动缓冲区指针
        data_pointer++;
        // 减少剩余要读取的字节数
        len--;
    }
    
    // 发送停止信号
    _iic_stop(para);

    return iic_success;
}

static void iic_sda_high(bsp_iic_obj_t *para)
{
    GPIO_SetBits(para->sda.port, para->sda.pin);
}

static void iic_sda_low(bsp_iic_obj_t *para)
{
    GPIO_ResetBits(para->sda.port, para->sda.pin);
}

static void iic_scl_high(bsp_iic_obj_t *para)
{
    GPIO_SetBits(para->scl.port, para->scl.pin);
}

static void iic_scl_low(bsp_iic_obj_t *para)
{
    GPIO_ResetBits(para->scl.port, para->scl.pin);
}

static uint8_t iic_read_sda(bsp_iic_obj_t *para)
{
    return GPIO_ReadInputDataBit(para->sda.port, para->sda.pin);
}

static bsp_iic_status_e _iic_start(bsp_iic_obj_t *para)
{
    iic_sda_high(para);
    iic_scl_high(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);

    if (iic_read_sda(para) == 0)
    {
        return iic_error_pull_push;
    }
    
    iic_sda_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
    
    iic_scl_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);

    return iic_success;
}

static void _iic_stop(bsp_iic_obj_t *para)
{
    iic_sda_low(para);
    iic_scl_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
    
    iic_scl_high(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
    
    iic_sda_high(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
}

static void _iic_ack(bsp_iic_obj_t *para)
{
    iic_sda_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
    
    iic_scl_high(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
    
    iic_scl_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS);
    
    iic_sda_high(para);
}

static void _iic_nack(bsp_iic_obj_t *para)
{
    iic_sda_high(para); // 拉高SDA线以生成NAck信号
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

    iic_scl_high(para); // 拉高SCL线，通知从设备NAck信号已发送
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保从设备可以检测到NAck信号

    // 等待从设备响应（可选）
    bsp_delay_ms(BSP_IIC_DELAY_MS);

    iic_scl_low(para); // 拉低SCL线，准备下一个操作
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

    // 确保SDA线在后续操作中保持高电平
    iic_sda_high(para);
}

static bsp_iic_status_e _iic_wait_ack(bsp_iic_obj_t *para)
{
    iic_sda_high(para);
    iic_scl_high(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

    if (iic_read_sda(para))
    {
        iic_scl_low(para);
        return iic_no_ack;
    }
    
    iic_scl_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

    return iic_success;
}

static void _iic_send_byte(bsp_iic_obj_t *para, uint8_t data)
{
    uint8_t index = 8;

    while (index--)
    {
        iic_scl_low(para);
        bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

        if (data & 0x80)
        {
            iic_sda_high(para);
        }
        else
        {
            iic_sda_low(para);
        }
        
        data <<= 1;
        bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

        iic_scl_high(para);
        bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定
    }

    iic_scl_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定
}

static uint8_t _iic_read_byte(bsp_iic_obj_t *para)
{
    uint8_t index = 8;
    uint8_t data = 0;

    iic_sda_high(para);
    while (index--)
    {
        data <<= 1;
        iic_scl_low(para);
        bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

        iic_scl_high(para);
        bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

        if (iic_read_sda(para))
        {
            data |= 0x01;
        }
        else
        {
            data |= 0x00;
        }
    }
    iic_scl_low(para);
    bsp_delay_ms(BSP_IIC_DELAY_MS); // 确保信号稳定

    return data;
}
