#include <rtthread.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "measure_data.h"
#include "XGZP6818D.h"
#include "sf_lib.h"
#include "first_order_filter.h"

static measure_blance_data_t blance;
static uint32_t blance_ticks;
static first_order_filter_para_t flow_filter_para;
static first_order_filter_data_t flow_filter_data_old, flow_filter_data_new;
static first_order_filter_t flow_filter;
float flow_value_use_original_sensor_value;
float flow_value_with_filter;
float flow_value_without_filter;
uint8_t printf_value_flag;

static measure_data_t flow_measure_data = 
{
    .flow_calib_data = 
    {
        {   .adc = 0,   .val = 0,   },
        {   .adc = 50,  .val = 50,  },
        {   .adc = 100, .val = 100, },
        {   .adc = 150, .val = 150, },
        {   .adc = 200, .val = 200, },
        {   .adc = 250, .val = 250, },
        {   .adc = 300, .val = 300, },
        {   .adc = 350, .val = 350, },
        {   .adc = 400, .val = 400, },
        {   .adc = 500, .val = 500, },
    },
    
    .flow_correction_data = 
    {
        .k = 1, .b = 0,
    },
};

//绝对温度
#define ABSOLUTETEMPERATURE(a)      (a + 273.15)
// 标准大气压
#define STANDARD_ATMOSPHERIC        101.325
#define SLIP_FILTER_NUM             5

void measure_flow_init(void)
{
    xgzp6818d_init();

    flow_filter_para.filter_damping_acc_value = 0.01;                  /* 消抖计数加速反应阀值*/
    flow_filter_para.fliter_dir_value = 0.001;                  /* 消抖计数加速反应阀值*/
    flow_filter_para.filter_damping_counter_max_value = 1;                  /* 消抖计数最大值(此常量影响滤波的灵敏度)*/
    flow_filter_para.filter_coefficient_add_value = 50;             /* 滤波系数增量(此常量影响滤波的灵敏度)*/
    flow_filter_para.filter_coefficient_max_value = 255;            /* 滤波系数最大值(此常量影响滤波的灵敏度)*/
    flow_filter_para.filter_coefficient_org_value = 0;              /* 滤波系数初始值 */
    flow_filter_para.filter_coefficient_variation_or_not = 0;       /* 滤波系数是否不变 0：动态变化 1：固定不变*/

    flow_filter.filter_para = &flow_filter_para;
    flow_filter.filter_data_old = &flow_filter_data_old;
    flow_filter.filter_data_new = &flow_filter_data_new;
    flow_filter.measure_data_num = 0;
}

void measure_flow_measure(measure_data_t *flow_data)
{
    static float pressure_array[SLIP_FILTER_NUM] = {0}, temp_array[SLIP_FILTER_NUM] = {0};
    static uint8_t array_index = 0;
    float perssure_temp = 0, tempewrature_temp = 0;
    float pressure_use_to_calculater = 0;
    float H = 0, P = 0, T = 0, V = 0;

    if (flow_data == NULL)
    {
        return;
    }

    if (xgzp6818d_read(&perssure_temp, &tempewrature_temp) != DATA_READ_SUCCESS)
    {
        return;
    }

    flow_data->pressure_ori = perssure_temp;
    flow_data->temp_ori = tempewrature_temp;

    pressure_array[array_index] = perssure_temp;
    temp_array[array_index] = tempewrature_temp;
    array_index++;
    
    // flow_data->average_perssure = average_float(pressure_array, array_index);
    // flow_data->average_temp = average_float(temp_array, array_index);
    flow_data->average_perssure = perssure_temp;
    flow_data->average_temp = tempewrature_temp;

    if (array_index >= SLIP_FILTER_NUM)
    {
        memmove(&pressure_array[0], &pressure_array[1], SLIP_FILTER_NUM * 4);
        memmove(&temp_array[0], &temp_array[1], SLIP_FILTER_NUM * 4);
        array_index = SLIP_FILTER_NUM;
    }

    flow_data->flow_sub_zero = flow_data->average_perssure - flow_data->flow_zero;
    if (flow_data->flow_sub_zero >= 0)
    {
        flow_data->flow_dir = 0;
    }
    else
    {
        flow_data->flow_dir = 1;
    }
    
    pressure_use_to_calculater = fabs(flow_data->flow_sub_zero);
    flow_filter.measure_data = pressure_use_to_calculater;
    H = first_older_filter_measure_data_append(&flow_filter);
    T = sqrtf(ABSOLUTETEMPERATURE(flow_data->average_temp) / ABSOLUTETEMPERATURE(20.0));    // 温度补偿系数
    P = 1.0;    // 压力补偿系数
    V = sqrtf((2 * H) / 1.293) * P * T;
    flow_data->flow_before_calib = V * 100;

    calib_data_calib(flow_data->flow_calib_data, 10, flow_data->flow_before_calib, &flow_data->flow_after_calib);
    calib_data_correction(&flow_data->flow_correction_data, flow_data->flow_after_calib, &flow_data->flow_correction);
    flow_data->flow_value = flow_data->flow_correction / 100;

    flow_value_without_filter = sqrtf((2 * fabs(pressure_use_to_calculater)) / 1.293) * P * T;
    calib_data_calib(flow_data->flow_calib_data, 10, flow_value_without_filter, &flow_value_without_filter);
    calib_data_correction(&flow_data->flow_correction_data, flow_value_without_filter, &flow_value_without_filter);

    flow_value_with_filter = flow_data->flow_value;
    flow_value_use_original_sensor_value = sqrtf((2 * fabs(flow_data->average_perssure)) / 1.293);
}

void measure_blance_start(uint8_t start_flag)
{
    blance.blance_flag = start_flag;
    blance.blance_tick = 0;
    memset(blance.blance_buffer, 0, 30 * 4);
    blance_ticks = rt_tick_get();
}

void measure_flow_set_zero(float *zero_data, float average_perssure_value, measure_blance_data_t *blance_data)
{

    blance_data->blance_buffer[blance_data->blance_tick] = average_perssure_value;
    blance_data->blance_tick++;
    if (blance_data->blance_tick >= 30)
    {
        bubble_sort_float(blance_data->blance_buffer, blance_data->blance_tick);

        *zero_data = average_float(blance_data->blance_buffer + 25, 4);  //去除最大值之后的四个最大值的平均值
        blance_data->blance_flag = 0;
    }
}

void measure_flow_hadnler(void)
{
    measure_flow_measure(&flow_measure_data);
    if (blance.blance_flag != 0)
    {
        if (rt_tick_get() - blance_ticks >= 1000)
        {
            measure_flow_set_zero(&flow_measure_data.flow_zero, flow_measure_data.average_perssure, &blance);
            blance_ticks = rt_tick_get();
        }
    }
}

/****************** 用于msh的设置与展示命令 ****************************/
#include <stdio.h>
#include <stdlib.h>

#pragma clang diagnostic ignored "-Wcast-function-type-mismatch"

extern char msh_buff[256];

void set_filter(int argc, char **argv)
{
    uint8_t cmd = 0;
    uint8_t value_u8 = 0;
    float value_f = 0;
    if (argc < 3)
    {
        rt_kprintf("parameters is too less\r\n");
        return;
    }
    
    cmd = atoi(argv[1]);
    if (cmd == 0x01 || cmd == 0x02)
    {
        value_f = atof(argv[2]);
    }
    else
    {
        value_u8 = atoi(argv[2]);
    }

    switch (cmd)
    {
    case 1:
        flow_filter_para.filter_damping_acc_value = value_f;
        break;
    
    case 2:
        flow_filter_para.fliter_dir_value = value_f;
        break;
    
    case 3:
        flow_filter_para.filter_damping_counter_max_value = value_u8;
        break;
    
    case 4:
        flow_filter_para.filter_coefficient_add_value = value_u8;
        break;
    
    case 5:
        flow_filter_para.filter_coefficient_max_value = value_u8;
        break;
    
    case 6:
        flow_filter_para.filter_coefficient_org_value = value_u8;
        break;
    
    case 7:
        flow_filter_para.filter_coefficient_variation_or_not = value_u8;
        break;
    
    default:
        break;
    }

    rt_kprintf("the fliter paraemters is :\r\n");
    memset(msh_buff, 0, 256);
    sprintf(msh_buff, "filter_damping_acc_value is %f\r\n", flow_filter_para.filter_damping_acc_value);
    rt_kprintf(msh_buff);
    sprintf(msh_buff, "fliter_dir_value is %f\r\n", flow_filter_para.fliter_dir_value);
    rt_kprintf(msh_buff);
    rt_kprintf("dmping_counter_max_value is: %d\r\n", flow_filter_para.filter_damping_counter_max_value);
    rt_kprintf("filter_coefficient_add_value is: %d\r\n", flow_filter_para.filter_coefficient_add_value);
    rt_kprintf("filter_coefficient_max_value is: %d\r\n", flow_filter_para.filter_coefficient_max_value);
    rt_kprintf("filter_coefficient_org_value is: %d\r\n", flow_filter_para.filter_coefficient_org_value);
    rt_kprintf("filter_coefficient_variation_or_not is: %d\r\n", flow_filter_para.filter_coefficient_variation_or_not);
    rt_kprintf("set cmd is: %d\r\n", cmd);
    rt_kprintf("/r/n");
}
MSH_CMD_EXPORT(set_filter, set the filter parameters);

void get_filter(void)
{
    rt_kprintf("the fliter paraemters is :\r\n");
    memset(msh_buff, 0, 256);
    rt_kprintf("set cmd is: %d\r\n", 1);
    sprintf(msh_buff, "filter_damping_acc_value is %f\r\n", flow_filter_para.filter_damping_acc_value);
    rt_kprintf(msh_buff);
    rt_kprintf("set cmd is: %d\r\n", 2);
    sprintf(msh_buff, "fliter_dir_value is %f\r\n", flow_filter_para.fliter_dir_value);
    rt_kprintf(msh_buff);
    rt_kprintf("set cmd is: %d\r\n", 3);
    rt_kprintf("dmping_counter_max_value is: %d\r\n", flow_filter_para.filter_damping_counter_max_value);
    rt_kprintf("set cmd is: %d\r\n", 4);
    rt_kprintf("filter_coefficient_add_value is: %d\r\n", flow_filter_para.filter_coefficient_add_value);
    rt_kprintf("set cmd is: %d\r\n", 5);
    rt_kprintf("filter_coefficient_max_value is: %d\r\n", flow_filter_para.filter_coefficient_max_value);
    rt_kprintf("set cmd is: %d\r\n", 6);
    rt_kprintf("filter_coefficient_org_value is: %d\r\n", flow_filter_para.filter_coefficient_org_value);
    rt_kprintf("set cmd is: %d\r\n", 7);
    rt_kprintf("filter_coefficient_variation_or_not is: %d\r\n", flow_filter_para.filter_coefficient_variation_or_not);
    
    rt_kprintf("/r/n");
}
MSH_CMD_EXPORT(get_filter, get the filter parameters);

void set_calib(int argc, char **argv)
{
    uint8_t index = 0;
    float adc = 0, val = 0;
    if (argc < 4)
    {
        rt_kprintf("parameters is too less\r\n");
        return;
    }
    
    index = atoi(argv[1]);
    if (index < 0 || index > 9)
    {
        rt_kprintf("parameters is illegal\r\n");
        rt_kprintf("parameters should in 0 - 9\r\n");
        return;
    }
    adc = atof(argv[2]);
    val = atof(argv[3]);
    flow_measure_data.flow_calib_data[index].adc = adc;
    flow_measure_data.flow_calib_data[index].val = val;

    rt_kprintf("the calib value is:\r\n");
    sprintf(msh_buff, "adc: %f, val: %f\r\n", flow_measure_data.flow_calib_data[index].adc, flow_measure_data.flow_calib_data[index].val);
    rt_kprintf(msh_buff);
}
MSH_CMD_EXPORT(set_calib, set calib value);

void get_calib(void)
{
    rt_kprintf("the calib value is:\r\n");
    for (int i = 0; i < 10; i++)
    {
        sprintf(msh_buff, "index: %d, adc: %f, val: %f\r\n", i, flow_measure_data.flow_calib_data[i].adc, flow_measure_data.flow_calib_data[i].val);
        rt_kprintf(msh_buff);
    }
}
MSH_CMD_EXPORT(get_calib, get calib value);

void set_corr(int argc, char **argv)
{
    float k = 0, b = 0;
    
    if (argc < 3)
    {
        rt_kprintf("parameters is too less\r\n");
        return;
    }
    k = atof(argv[1]);
    b = atof(argv[2]);
    flow_measure_data.flow_correction_data.k = k;
    flow_measure_data.flow_correction_data.b = b;
    sprintf(msh_buff, "correction value is k: %f, b: %f\r\n", flow_measure_data.flow_correction_data.k, flow_measure_data.flow_correction_data.b);
    rt_kprintf(msh_buff);
}
MSH_CMD_EXPORT(set_corr, set correction value);

void get_corr(void)
{
    sprintf(msh_buff, "correction value is k: %f, b: %f\r\n", flow_measure_data.flow_correction_data.k, flow_measure_data.flow_correction_data.b);
    rt_kprintf(msh_buff);
}
MSH_CMD_EXPORT(get_corr, get correction value);

void reset_para(void)
{
    for (int i = 0; i < 9; i++)
    {
        flow_measure_data.flow_calib_data[i].adc = i * 50;
        flow_measure_data.flow_calib_data[i].val = i * 50;
    }
    flow_measure_data.flow_calib_data[9].adc = 500;
    flow_measure_data.flow_calib_data[9].val = 500;

    flow_measure_data.flow_correction_data.k = 1;
    flow_measure_data.flow_correction_data.b = 0;
    rt_kprintf("the calib value is:\r\n");
    for (int i = 0; i < 10; i++)
    {
        sprintf(msh_buff, "index: %d, adc: %f, val: %f\r\n", i, flow_measure_data.flow_calib_data[i].adc, flow_measure_data.flow_calib_data[i].val);
        rt_kprintf(msh_buff);
    }
    rt_kprintf("/r/n");
    sprintf(msh_buff, "correction value is k: %f, b: %f\r\n", flow_measure_data.flow_correction_data.k, flow_measure_data.flow_correction_data.b);
    rt_kprintf(msh_buff);
}
MSH_CMD_EXPORT(reset_para, reset calib and correction value);

void show_value(void)
{
    sprintf(msh_buff, "flow_value_with_filter is %f\r\n", flow_value_with_filter);
    rt_kprintf(msh_buff);
    sprintf(msh_buff, "flow_value_without_filter is %f\r\n", flow_value_without_filter);
    rt_kprintf(msh_buff);
    sprintf(msh_buff, "flow_value_use_original_sensor_value is %f\r\n", flow_value_use_original_sensor_value);
    rt_kprintf(msh_buff);
}
MSH_CMD_EXPORT(show_value, show measure value);

void printf_start(void)
{
    printf_value_flag = 1;
    rt_kprintf("start printf measure value\r\n");
}
MSH_CMD_EXPORT(printf_start, start printf measure value);

void printf_stop(void)
{
    printf_value_flag = 0;
    rt_kprintf("stop printf measure value\r\n");
}
MSH_CMD_EXPORT(printf_stop, stop printf measure value);
