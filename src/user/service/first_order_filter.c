#include "first_order_filter.h"
#include <math.h>
#include <string.h>

static float first_order_filter_computation_result(first_order_filter_t *para);
static void _data_add_with_boundary_judgment(void *data_pointer, uint32_t max_value);

/*
*********************************************************************************************************
* 动态滤波原理:
* ==============
* 在进行调整之前，我们需要先进行以下判断：
* a)  数据变化是否朝向同一个方向（比如，当连续两次的采样值都比其上次滤波结果大，视为变化方向一致，否则视为不一致）；
* b)  数据变化是否较快（主要是判断采样值和上次滤波结果之间的差值）。
* ●  调整的原理如下：
* a)  当两次数据变化方向不一致时，说明有抖动，将滤波系数清零，忽略本次新采样值；
* b)  当数据持续向一个方向变化时，逐渐提高滤波系数，提高本次新采样值的权；
* c)  当数据变化较快(差值>消抖计数加速反应阀值)时，要加速提高滤波系数。
*********************************************************************************************************
*/

float first_older_filter_measure_data_append(first_order_filter_t *para)
{
    float result = 0;
    float new_sub_old = 0;

    _data_add_with_boundary_judgment(&para->measure_data_num, 0xffffffff);

    if (para->measure_data_num == 1)
    {
        para->filter_data_old->filter_data_old = para->measure_data;
        para->filter_data_old->filter_data_new = para->measure_data;
        para->filter_data_old->filter_coefficient_last = para->filter_para->filter_coefficient_org_value;
        para->filter_data_old->filter_data_add_or_dec = 0;
    }
    else
    {
        if (para->measure_data_num > 2)         //存在上一次的数据，并非刚开始进行滤波
        {
            memcpy(para->filter_data_old, para->filter_data_new, sizeof(first_order_filter_data_t));
        }
        
        para->filter_data_new->filter_data_old = para->filter_data_old->filter_data_new;
        para->filter_data_new->filter_data_new = para->measure_data;
        para->filter_data_new->filter_coefficient_last = para->filter_para->filter_coefficient_org_value;
        new_sub_old = para->filter_data_new->filter_data_new - para->filter_data_new->filter_data_old;
        if (new_sub_old > 0 && fabs(new_sub_old) > para->filter_para->fliter_dir_value)
        {
            para->filter_data_new->filter_data_add_or_dec = 1;
        }
        else if (new_sub_old < 0 && fabs(new_sub_old) > para->filter_para->fliter_dir_value)
        {
            para->filter_data_new->filter_data_add_or_dec = 2;
        }
        else
        {
            para->filter_data_new->filter_data_add_or_dec = 0;
        }
        
        if (para->measure_data_num > 2)         //返回滤波数据
        {
            result= first_order_filter_computation_result(para);
        }
        else
        {
            result = para->filter_data_new->filter_data_new;
        }
    }
    return result;
}

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
static float first_order_filter_computation_result(first_order_filter_t *para)
{
    float result = 0;
    float new_sub_old = 0;
    new_sub_old = para->filter_data_new->filter_data_new - para->filter_data_new->filter_data_old;
    if (para->filter_para->filter_coefficient_variation_or_not == 0)   //动态系数滤波
    {
        //两次变化一致
        if (para->filter_data_new->filter_data_add_or_dec == para->filter_data_old->filter_data_add_or_dec)
        {
            _data_add_with_boundary_judgment(&para->filter_damping_counter, 0xff);

            if (fabs(new_sub_old) > para->filter_para->filter_damping_acc_value)
            {
                _data_add_with_boundary_judgment(&para->filter_damping_counter, 0xff);
            }
            
            if (para->filter_damping_counter >= para->filter_para->filter_damping_max_value)
            {
                para->filter_data_new->filter_coefficient_last = para->filter_data_old->filter_coefficient_last + para->filter_para->filter_coefficient_add_value;
                if (para->filter_data_new->filter_coefficient_last >= para->filter_para->filter_coefficient_max_value)
                {
                    para->filter_data_new->filter_coefficient_last = para->filter_para->filter_coefficient_max_value;
                }
                para->filter_damping_counter = 0;
            }
        }
        else
        {
            para->filter_damping_counter = 0;
            para->filter_data_new->filter_coefficient_last = para->filter_para->filter_coefficient_org_value;
        }
        
        //使用新的滤波系数进行计算
        if (para->filter_data_new->filter_data_add_or_dec != 0)
        {
            result = para->filter_data_new->filter_data_old + new_sub_old * para->filter_data_new->filter_coefficient_last / 256;
        }
        else
        {
            // 新旧数据无变化，所以不需要采用滤波
            result = para->filter_data_new->filter_data_old;
        }
    }
    else    //固定系数滤波
    {
        if (para->filter_data_new->filter_data_add_or_dec != 0)
        {
            result = para->filter_data_new->filter_data_old + new_sub_old * para->filter_para->filter_coefficient_org_value / 256;
        }
        else
        {
            // 新旧数据无变化，所以不需要采用滤波
            result = para->filter_data_new->filter_data_old;
        }
    }
    para->filter_data_new->filter_data_new = result;
    return result;
}

//使用最大值限制当前数值的增加
static void _data_add_with_boundary_judgment(void *data_pointer, uint32_t max_value)
{
    if (max_value <= 0xffffffff && max_value > 0xffff)
    {
        if (*(uint32_t*)data_pointer < max_value)
        {
            *(uint32_t*)data_pointer = *(uint32_t*)data_pointer + 1;
        }
    }
    else if (max_value <= 0xffff && max_value > 0xff)
    {
        if (*(uint16_t*)data_pointer < max_value)
        {
            *(uint16_t*)data_pointer = *(uint16_t*)data_pointer + 1;
        }
    }
    else if (max_value <= 0xff && max_value > 0x00)
    {
        if (*(uint8_t*)data_pointer < max_value)
        {
            *(uint8_t*)data_pointer = *(uint8_t*)data_pointer + 1;
        }
    }
    else
    {
        (void)data_pointer;
    }
}
