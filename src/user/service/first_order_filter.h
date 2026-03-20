#ifndef _FIRST_ORDER_FILTER_H__
#define _FIRST_ORDER_FILTER_H__

#include <stdint.h>

/*
*********************************************************************************************************
* 一阶滤波，又叫一阶惯性滤波，或一阶低通滤波。是使用软件编程实现普通硬件RC低通滤波器的功能。
* 一阶低通滤波的算法公式为：Y(n)=αX(n)+(1-α)Y(n-1) 
* 式中：α=滤波系数；X(n)=本次采样值；Y(n-1)=上次滤波输出值；Y(n)=本次滤波输出值。
* 一阶低通滤波法采用本次采样值与上次滤波输出值进行加权，得到有效滤波值，使得输出对输入有反馈作用。
* 公式原型：
* 本次滤波结果＝新采样值×滤波系数÷256＋上次滤波结果×（256－滤波系数）÷256 
* 滤波系数＝0～255；该系数决定新采样值在本次滤波结果中所占的权重。
* 滤波系数越小，滤波结果越平稳，但是灵敏度越低；滤波系数越大，灵敏度越高，但是滤波结果越不稳定。
* 所以最终优化后的版本 Y(n)=αX(n)/256+(256-α)/256*Y(n-1) 
* Y(n)=Y(n-1) + α/256*(X(n) - Y(n-1))
*********************************************************************************************************
*/

typedef struct
{
    float       filter_damping_acc_value;                   /* 消抖计数加速反应阀值*/
    float       fliter_dir_value;                           /* 用于判断是否属于数据变化的阀值，小于此值的变化视为没有变化 */
    uint8_t     filter_damping_max_value;                   /* 消抖计数最大值(此常量影响滤波的灵敏度)*/
    uint8_t     filter_coefficient_add_value;               /* 滤波系数增量(此常量影响滤波的灵敏度)*/
    uint8_t     filter_coefficient_max_value;               /* 滤波系数最大值(此常量影响滤波的灵敏度)*/
    uint8_t     filter_coefficient_org_value;               /* 滤波系数初始值 */
    uint8_t     filter_coefficient_variation_or_not;        /* 滤波系数是否不变 0：动态变化 1：固定不变*/
}first_order_filter_para_t;

typedef struct
{
    float   filter_data_new;                /*新采样值*/
    float   filter_data_old;                /*上次滤波结果*/
    uint16_t  filter_coefficient_last;      /*上次滤波系数(0~255)*/
    uint8_t   filter_data_add_or_dec;       /*上次数据变化方向标志(0=不变，1=增加，2=减少)*/
}first_order_filter_data_t;

typedef struct
{
    first_order_filter_para_t   *filter_para;           /*一阶滤波参数*/
    first_order_filter_data_t   *filter_data_old;       /*旧数据，用于进行滤波计算*/
    first_order_filter_data_t   *filter_data_new;       /*新数据，用于进行滤波计算*/
    uint8_t                     filter_damping_counter; /*滤波消抖计数器*/
    uint32_t                    measure_data_num;       /*总测量数据个数*/
    float                       measure_data;           /*当前需要参与运算的测量数据，在运算完毕后变成运算后的数据*/
}first_order_filter_t;

float first_older_filter_measure_data_append(first_order_filter_t *para);

#endif
