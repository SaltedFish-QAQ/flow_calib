#include <rtthread.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "thread_config.h"

#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"

static void gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // 使能PB端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;        // LED0-->PB.5 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);            // 根据设定参数初始化GPIOB.5
    GPIO_SetBits(GPIOC, GPIO_Pin_13);                 // PB.5 输出高
}

void hello(void)
{
    rt_kprintf("hello rtthread nano\r\n");
}
MSH_CMD_EXPORT(hello, say hello to RT-Thread nano);

float test_float1, test_float2;
void set_float(int argc, char **argv)
{
    if (argc < 3)
    {
        rt_kprintf("parameters is too less\r\n");
        return;
    }
    
    test_float1 = atof(argv[1]);
    test_float2 = atof(argv[2]);
    rt_kprintf("the input parameters is %.2f, %.2f\r\n", test_float1, test_float2);
}
MSH_CMD_EXPORT(set_float, input float parameters);

void get_float(void)
{
    rt_kprintf("the float parameters is %.2f, %.2f\r\n", test_float1, test_float2);
}
MSH_CMD_EXPORT(get_float, output float parameters);

int main(void)
{
    gpio_init();
    flow_calib_thread_create();
    flow_calib_thread_run();

    while (1)
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        rt_thread_mdelay(500);
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        rt_thread_mdelay(500);
    }
}
