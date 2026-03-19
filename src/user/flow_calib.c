#include <rtthread.h>
#include "thread_config.h"
#include "measure_data.h"

static struct rt_thread flow_calib_thread;
static char flow_calib_thread_stack[FLOW_CALIB_STACK_SIZE];

static void flow_calib_thread_entry(void *param)
{
    measure_flow_init();
    measure_blance_start(1);
    while (1)
    {
        measure_flow_hadnler();
        rt_thread_mdelay(100);
    }
    
}

void flow_calib_thread_create(void)
{
    rt_thread_init(&flow_calib_thread,
                   "flow_calib_thread",
                   flow_calib_thread_entry,
                   RT_NULL,
                   &flow_calib_thread_stack[0],
                   sizeof(flow_calib_thread_stack),
                   FLOW_CALIB_PRIORITY, FLOW_CALIB_TIMESLICE);
}

void flow_calib_thread_run(void)
{
    rt_thread_startup(&flow_calib_thread);
}
