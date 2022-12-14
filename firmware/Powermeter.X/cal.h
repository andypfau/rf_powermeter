#ifndef CAL_H
#define	CAL_H


#include <stdint.h>


void cal_init(void);
void cal_load(uint16_t f_mhz);
int cal_get_mhz();
int32_t cal_apply(int32_t reading_mdb);


#endif	/* CAL_H */
