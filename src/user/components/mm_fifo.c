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
#include "mm_fifo.h"
#include <string.h>
struct _MM_FIFO
{

    size_t out; /*!< 唤醒队列的数据起始地址 */
    size_t in;  /*!< 有效数据长度 */
    // 元素大小.单位: 字节
    size_t size;    /*!< 缓存区大小 */
    uint8_t data[]; /*!< 缓存区起始地址 */
};
#define INDEX_NEXT(self, offset, idx) ((idx + offset) % self->size)
// #define INDEX_LAST(self, offset, idx) ((idx - offset) % self->size)
mm_fifo_t *mm_fifo_init(void *data_ptr, size_t data_size)
{
    if (data_size <= sizeof(mm_fifo_t))
    {
        return NULL;
    }
    mm_fifo_t *self = (mm_fifo_t *)data_ptr;
    self->size = data_size - sizeof(mm_fifo_t);
    self->out = 0;
    self->in = 0;
    return self;
}
bool mm_fifo_is_empty(mm_fifo_t *self)
{
    return self->in == self->out;
}
bool mm_fifo_is_full(mm_fifo_t *self)
{
    return INDEX_NEXT(self, 1, self->in) == self->out;
}
bool mm_fifo_push(mm_fifo_t *self, uint8_t dat)
{
    size_t next_in = INDEX_NEXT(self, 1, self->in);
    if (next_in != self->out)
    {
        self->data[self->in] = dat;
        self->in = next_in;
        return true;
    }
    return false;
}
uint8_t mm_fifo_pop(mm_fifo_t *self)
{
    uint8_t data_temp = 0;
    if (!mm_fifo_is_empty(self))
    {
        data_temp = self->data[self->out];
        self->out = INDEX_NEXT(self, 1, self->out);
    }
    return data_temp;
}
uint8_t mm_fifo_pop_peek(mm_fifo_t *self)
{
    uint8_t value;
    mm_fifo_t self_bak = *self;
    value = mm_fifo_pop(self);
    *self = self_bak;
    return value;
}
size_t mm_fifo_push_multi(mm_fifo_t *self, uint8_t *dat, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
    {
        if (mm_fifo_push(self, dat[i]) == 0)
            break;
    }
    return i;
}
size_t mm_fifo_push_multi_force(mm_fifo_t *self, uint8_t *dat, size_t data_size)
{
    if (data_size > self->size)
    {
        data_size = self->size;
    }
    size_t free_space = mm_fifo_get_unused_space(self);

    if (free_space < data_size)
    {
        mm_fifo_pop_quick(self, data_size - free_space);
    }

    size_t i;
    if (self->size - self->in > data_size)
    {
        memcpy(self->data + self->in, dat, data_size);
        self->in += data_size;
    }else{
        memcpy(self->data + self->in, dat, self->size - self->in);
        memcpy(self->data , dat, self->size - self->in);
    }
    for (i = 0; i < data_size; i++)
    {
        if (mm_fifo_push(self, dat[i]) == 0)
            break;
    }
        return data_size;

}
size_t mm_fifo_pop_multi(mm_fifo_t *self, uint8_t *dat, size_t size)
{
    size_t i;
    size_t used = mm_fifo_get_used_space(self);
    if (used < size)
    {
        size = used;
    }
    for (i = 0; i < size; i++)
    {
        dat[i] = mm_fifo_pop(self);
    }
    return size;
}
/**
 * @brief 取出多个数据，不从队列中移除
 * @param self 队列的句柄
 * @param dat 取出数据存放空间
 * @param size 要取出的数据
 * @return 实际取出来的数据
 */
size_t mm_fifo_pop_multi_peek(mm_fifo_t *self, uint8_t *dat, size_t size)
{
    mm_fifo_t self_bak = *self;
    size_t len = mm_fifo_pop_multi(self, dat, size);
    *self = self_bak;
    return len;
}
size_t mm_fifo_get_valid_data_peek(mm_fifo_t *self, uint8_t **date_ptr)
{
    *date_ptr = &self->data[self->out];
    if (self->size > self->out + self->in)
    {
        return self->in;
    }
    else
    {
        return self->size - self->out;
    }
}
size_t mm_fifo_get_free_data_peek(mm_fifo_t *self, uint8_t **date_ptr)
{
    size_t end = self->in;
    *date_ptr = &self->data[end];
    if (end > self->out)
    {
        return self->size - end;
    }
    else
    {
        return self->out - end;
    }
}
size_t mm_fifo_get_used_space(mm_fifo_t *self)
{
    size_t cnt;
    if (self->in < self->out)
    {
        cnt = self->size - (self->out - self->in);
    }
    else
    {
        cnt = self->in - self->out;
    }

    return cnt;
}
size_t mm_fifo_get_unused_space(mm_fifo_t *self)
{
    size_t cnt;
    if (self->in < self->out)
    {
        cnt = (self->out - self->in);
    }
    else
    {
        cnt = self->size - (self->in - self->out);
    }
    return cnt;
}

void mm_fifo_reset(mm_fifo_t *self)
{
    self->out = 0;
    self->in = 0;
}

size_t mm_fifo_pop_quick(mm_fifo_t *self, size_t cnt)
{
    size_t len = mm_fifo_get_used_space(self);
    if (len < cnt)
    {
        cnt = len;
    }
    self->out = INDEX_NEXT(self, cnt, self->out);
    return cnt;
}

size_t mm_fifo_push_quick(mm_fifo_t *self, size_t cnt)
{
    size_t len = mm_fifo_get_unused_space(self);
    if (len > cnt)
    {
        self->in = self->in + cnt;
        return cnt;
    }
    else
    {
        self->in = self->size;
        return len - 1;
    }
}
