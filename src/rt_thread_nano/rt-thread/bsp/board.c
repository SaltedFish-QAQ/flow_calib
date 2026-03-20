/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-24                  the first version
 */
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>

#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))

// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;

static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  
    
    return 0;
}

#ifdef RT_USING_CONSOLE
static int uart_init(void);
#endif

#ifdef RT_DEBUG
#include "cm_backtrace.h"

static rt_err_t exception_hook(void *context) {
    extern long list_thread(void);
    uint8_t _continue = 1;

    rt_enter_critical();

#ifdef RT_USING_FINSH
    list_thread();
#endif

    /* 建立深度为 16 的函数调用栈缓冲区，深度大小不应该超过 CMB_CALL_STACK_MAX_DEPTH（默认16） */
    uint32_t call_stack[16] = {0};
    size_t i, depth = 0;
    /* 获取当前环境下的函数调用栈，每个元素将会以 32 位地址形式存储， depth 为函数调用栈实际深度 */
    depth = cm_backtrace_call_stack(call_stack, sizeof(call_stack), cmb_get_sp());
    /* 输出当前函数调用栈信息
    * 注意：查看函数名称及具体行号时，需要使用 addr2line 工具转换
    */
    for (i = 0; i < depth; i++) {
        cmb_println("%08x ", call_stack[i]);
    }

    while (_continue == 1);

    return RT_EOK;
}
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
/*
 * Please modify RT_HEAP_SIZE if you enable RT_USING_HEAP
 * the RT_HEAP_SIZE max value = (sram size - ZI size), 1024 means 1024 bytes
 */
#define RT_HEAP_SIZE (15*1024)
static rt_uint8_t rt_heap[RT_HEAP_SIZE];

RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

void rt_os_tick_callback(void)
{
    rt_interrupt_enter();
    
    rt_tick_increase();

    rt_interrupt_leave();
}

void SysTick_Handler(void)
{
    rt_os_tick_callback();
}

/**
 * This function will initial your board.
 */

void rt_hw_board_init(void)
{
    /* 
     * TODO 1: OS Tick Configuration
     * Enable the hardware timer and call the rt_os_tick_callback function
     * periodically with the frequency RT_TICK_PER_SECOND. 
     */
    /* System Clock Update */
    SystemCoreClockUpdate();
    
    /* System Tick Configuration */
    _SysTick_Config(SystemCoreClock / RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
    #ifdef RT_USING_CONSOLE
    uart_init();
#ifdef RT_DEBUG
    cm_backtrace_init("flow_calib.axf", "0.01", "0.01");
    rt_hw_exception_install(exception_hook);
#endif
    #endif
}

#ifdef RT_USING_CONSOLE

#include "bsp_usart.h"

static int uart_init(void)
{
    bsp_usart_finsh_init();
    return 0;
}

#endif

