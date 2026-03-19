/**
 * @file mm_fifo.h
 * COPYRIGHT (C) 2024,  chengmeng_2@outlook.com Co., Ltd.
 * All rights reserved.
 * @brief 环形队列管理器
 * @details 微型环形队列管理器，用于单片机等场景
 *  在串口数据首发中较为常用
 * @version ver2.0
 * @date 2024年2月27日
 * @author cmm
 * @note
 */

#ifndef __MM_FIFO_H_
#define __MM_FIFO_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef struct _MM_FIFO mm_fifo_t;

    /**
     * @brief 初始化环形队列空间
     * @note 为了更好的兼容应用场景，内部不做内存申请 
     * @param self object 队列句柄
     * @param data_ptr 缓存区地址
     * @param data_size 缓存区大小
     */
    mm_fifo_t *mm_fifo_init(void *data_ptr, size_t data_size);
    /**
     * @brief 判断是否为空
     * @param self 队列的句柄
     * @return true:队列为空 false 不空
     */
    bool mm_fifo_is_empty(mm_fifo_t *self);
    /**
     * @brief 判断是否为满
     * @param self 队列的句柄
     * @return true:队列为满 false 不满
     */
    bool mm_fifo_is_full(mm_fifo_t *self);
    /**
     * @brief 存入一个数据
     * @param self 队列的句柄
     * @param dat 存入的数据
     * @return true:存入成功 false 失败
     */
    bool mm_fifo_push(mm_fifo_t *self, uint8_t dat);
    /**
     * @brief 取出一个数据
     * @note 未做空判定，用户自行调用
     * @param self 队列的句柄
     * @return 取出的数据
     */
    uint8_t mm_fifo_pop(mm_fifo_t *self);
    /**
     * @brief 取出一个数据
     * @note 取出数据,不从队列中清除
     * @param self 队列的句柄
     * @return 待取出的数据
     */
    uint8_t mm_fifo_pop_peek(mm_fifo_t *self);
    /**
     * @brief 存入多个数据
     * @param self 队列的句柄
     * @param dat 待存入的数据
     * @param data_size 数据数量
     * @return 实际存进去的数据数量
     */
    size_t mm_fifo_push_multi(mm_fifo_t *self, uint8_t *dat, size_t data_size);

    /**
     * @brief 强制存入数据，如果空间不足将覆盖现有数据
     * @param self 队列的句柄
     * @param dat 待存入的数据
     * @param data_size 数据数量
     * @return 实际存进去的数据数量
     */
    size_t mm_fifo_push_multi_force(mm_fifo_t *self, uint8_t *dat, size_t data_size);

    /**
     * @brief 取出多个数据
     * @param self 队列的句柄
     * @param dat 取出数据存放空间
     * @param data_size 要取出的数据
     * @return 实际取出来的数据
     */
    size_t mm_fifo_pop_multi(mm_fifo_t *self, uint8_t *dat, size_t data_size);
    /**
     * @brief 取出多个数据，不从队列中移除
     * @param self 队列的句柄
     * @param dat 取出数据存放空间
     * @param data_size 要取出的数据
     * @return 实际取出来的数据
     */
    size_t mm_fifo_pop_multi_peek(mm_fifo_t *self, uint8_t *dat, size_t data_size);
    /**
     * @brief 得到待取出数据的连续内存段，不从队列中移除
     * @param self 队列的句柄
     * @param date_ptr 有效数据的起始地址
     * @return 有效数据的长度
     */
    size_t mm_fifo_get_valid_data_peek(mm_fifo_t *self, uint8_t **date_ptr);
    /**
     * @brief 得到空数据的连续内存段，不从队列中移除
     * @param self 队列的句柄
     * @param date_ptr 空的起始地址
     * @return 空数据的最大长度
     */
    size_t mm_fifo_get_free_data_peek(mm_fifo_t *self, uint8_t **date_ptr);
    /**
     * @brief 未使用空间
     * @param self 队列的句柄
     * @return 队列中元素的数量
     */
    size_t mm_fifo_get_used_space(mm_fifo_t *self);
    /**
     * @brief 使用的空间
     * @param self 队列的句柄
     * @return 队列中空闲的数量
     */
    size_t mm_fifo_get_unused_space(mm_fifo_t *self);
    /**
     * @brief 清空fifo
     * @param self 队列的句柄
     */
    void mm_fifo_reset(mm_fifo_t *self);
    /**
     * @brief fifo快速弹出不要的数据
     * @param self 队列的句柄
     * @param cnt 要弹出的数量
     * @return 实际弹出的数量
     */
    size_t mm_fifo_pop_quick(mm_fifo_t *self, size_t cnt);
    /**
     * @brief fifo快速压入的数据，可以提前占用空间
     * @param self 队列的句柄
     * @param cnt 要压入的数量
     * @return 实际压入的数量
     */
    size_t mm_fifo_push_quick(mm_fifo_t *self, size_t cnt);

#ifdef __cplusplus
}
#endif

#endif //__MM_FIFO_H_
