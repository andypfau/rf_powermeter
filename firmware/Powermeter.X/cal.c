#include "cal.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>


int find_closest_lower_idx(int16_t needle, int16_t haystack[], int haystack_size)
{
    int best_idx = 0;
    int16_t best_err = 0x7FFF;
    for (int i = 0; i < haystack_size; i++) {
        int16_t err = abs(needle - haystack[i]);
        if (err < best_err) {
            best_idx = i;
            best_err = err;
        }
    }
    return best_idx;
}


uint16_t interpolate(int16_t x, int32_t slope, int32_t offset, int varShift, int facShift)
{
    int32_t xw = ((int32_t)x) << varShift;
    int32_t yw = offset + slope * xw;
    return (int16_t)(yw >> (varShift+facShift));
}


void cal_init(void)
{
    // TODO
}


int16_t get_cal(int16_t f)
{
    /*int idx = find_closest_lower_idx(f, Frequency_MHz, N_FREQUENCIES);
    return interpolate(f, Errors_mdB_Slope[idx], Errors_mdB_Offset[idx], VAR_SHIFT, SLOPE_SHIFT);*/
    return 0; // TODO
}
