#ifndef _XGZP6818D_H_
#define _XGZP6818D_H_

#include <stdint.h>

#define DATA_READ_SUCCESS       0x00
#define DATA_NOT_READY          0xff

uint8_t xgzp6818d_init(void);
uint8_t xgzp6818d_read(float *pressure, float *temp);

#endif
