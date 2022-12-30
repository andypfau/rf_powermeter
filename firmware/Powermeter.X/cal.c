#include "cal.h"
#include "memory.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>


#define MEM_ADDR_CAL_DATA_COUNT        0x0000
#define MEM_ADDR_CAL_VAR_SHIFT         0x0002
#define MEM_ADDR_CAL_SLOPE_SHIFT       0x0004
#define MEM_ADDR_CAL_FREQ_DATA_START   0x4000
#define MEM_ADDR_CAL_SLOPE_DATA_START  0x8000
#define MEM_ADDR_CAL_OFFSET_DATA_START 0xC000


static uint16_t NumberOfEntries;
static int VarShift, SlopeShift;
static int32_t Slope, Offset;
static uint16_t MHz;
static int32_t ErrorToCompensate;


uint16_t read_uint16(int address)
{
    uint16_t buffer;
    mem_read(address, 2, (uint8_t*)(&buffer));
    mem_wait();
    return buffer;
}


int32_t read_int32(int address)
{
    uint32_t buffer;
    mem_read(address, 4, (uint8_t*)(&buffer));
    mem_wait();
    uint32_t fixed = (buffer << 16) | (buffer >> 16); // fix 16b-word endianness
    return (int32_t)fixed;
}


void load_offset_and_slope(uint16_t f_mhz)
{
    long lowestIndex = 0;
    long highestIndex = NumberOfEntries-1;
    long currentIndex = (lowestIndex + highestIndex) >> 1;
    
    // find the lowest index that is less than or equal to the sought frequency
    // using binary search, because linear search is super slow
    while (highestIndex > lowestIndex) {
        
        uint16_t f_mhz_mem = read_uint16(MEM_ADDR_CAL_FREQ_DATA_START + currentIndex*2);
        
        if (f_mhz_mem == f_mhz) {
            // found it
            break;
        } else if (f_mhz_mem > f_mhz) {
            // too high, go lower
            highestIndex = currentIndex;
        } else {
            // too low, go higher
            lowestIndex = currentIndex + 1;
        }
        currentIndex = (lowestIndex + highestIndex) >> 1;
    }
    if (currentIndex != 0)
        currentIndex--; // we need the next *lower* index for proper interpolation
    
    Slope  = read_int32(MEM_ADDR_CAL_SLOPE_DATA_START  + currentIndex*4);
    Offset = read_int32(MEM_ADDR_CAL_OFFSET_DATA_START + currentIndex*4);
}


int32_t interpolate(uint16_t x, int32_t slope, int32_t offset, int varShift, int facShift)
{
    int32_t xw = ((int32_t)x) << varShift;
    int32_t yw = offset + slope * xw;
    return yw >> (varShift + facShift);
}


void cal_init(void)
{
    NumberOfEntries = read_uint16(MEM_ADDR_CAL_DATA_COUNT);
    VarShift        = read_uint16(MEM_ADDR_CAL_VAR_SHIFT);
    SlopeShift      = read_uint16(MEM_ADDR_CAL_SLOPE_SHIFT);
    
    cal_load(1000);
}


void cal_load(uint16_t f_mhz)
{
    load_offset_and_slope(f_mhz);
    ErrorToCompensate = interpolate(f_mhz, Slope, Offset, VarShift, SlopeShift);
    MHz = f_mhz;
}


int cal_get_mhz(void)
{
    return MHz;
}


int32_t cal_apply(int32_t reading_mdb)
{
    return reading_mdb - ErrorToCompensate;
}
