#include "cal.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>


#define VAR_SHIFT 1
#define SLOPE_SHIFT 8


#define MEM_ADDR_CAL_DATE 0
#define MEM_ADDR_CAL_TEMP 4
#define MEM_ADDR_CAL_DATA_COUNT 6
#define MEM_ADDR_CAL_FREQ_DATA_START 8
#define MEM_ADDR_CAL_SLOPE_DATA_START 184
#define MEM_ADDR_CAL_OFFSET_DATA_START 532
#define MEM_ADDR_CAL_DATA_END 880


static int NumberOfEntries;
static uint32_t Slope, Offset;


int32_t interpolate(int16_t x, int32_t slope, int32_t offset, int varShift, int facShift)
{
    int32_t xw = ((int32_t)x) << varShift;
    int32_t yw = offset + slope * xw;
    return yw >> (varShift + facShift);
}


void cal_init(void)
{
    Slope = 0;
    Offset = 0;
    
    mem_read(MEM_ADDR_CAL_DATA_COUNT, 2, (uint8_t*)(&NumberOfEntries));
    mem_wait();
}


void cal_load(int16_t f_mhz)
{
    // find closest address
    int best_idx = 0;
    int16_t best_err = 0x7FFF;
    for (int i = 0; i < NumberOfEntries; i++) {
        
        uint16_t f_mhz_mem;
        mem_read(MEM_ADDR_CAL_FREQ_DATA_START + i*2, 2, (uint8_t*)(&f_mhz_mem));
        mem_wait();
        
        int16_t err = abs(f_mhz - f_mhz_mem);
        if (err < best_err) {
            best_idx = i;
            best_err = err;
        }
    }
    
    // load cal data
    mem_read(MEM_ADDR_CAL_SLOPE_DATA_START + best_idx*4, 4, (uint8_t*)(&Slope));
    mem_wait();
    mem_read(MEM_ADDR_CAL_OFFSET_DATA_START + best_idx*4, 4, (uint8_t*)(&Offset));
    mem_wait();
}


int32_t cal_apply(int32_t reading_mdb)
{
    return NumberOfEntries;
    //return interpolate(reading_mdb, Slope, Offset, VAR_SHIFT, SLOPE_SHIFT);
}
