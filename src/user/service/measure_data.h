#ifndef _MEASURE_DATA_H_
#define _MEASURE_DATA_H_

#include <stdint.h>
#include "calib.h"

typedef struct
{
    calib_data_t flow_calib_data[10];       //标定数据
    correction_data_t flow_correction_data; //修正数据
    
    float pressure_ori;                     // 传感器差压原始值
    float temp_ori;                         // 传感器温度原始值
    float flow_zero;                        // 零点标定值
    float flow_sub_zero;                    // 经过减去零点之后的值
    float flow_before_calib;                // 标定之前的值，标定使用
    float flow_after_calib;                 // 经过标定后的值
    float flow_correction;                  // 经过修正之后的值
    float flow_value;                        // 最终显示值
    float average_perssure;                 // 经过滑差滤波后的差压值
    float average_temp;                     // 经过滑差滤波后的温度值
    uint8_t flow_dir;                       // 流速方向, 0 差压传感器测量值为正，1 差压传感器测量为负
}measure_data_t;

typedef struct
{
    float blance_buffer[30];
    uint8_t blance_tick;
    uint8_t blance_flag;
}measure_blance_data_t;

void measure_flow_init(void);

void measure_flow_measure(measure_data_t *flow_data);

void measure_blance_start(uint8_t start_flag);
void measure_flow_set_zero(float *zero_data, float average_perssure_value, measure_blance_data_t *blance_data);

void measure_flow_hadnler(void);

#endif
