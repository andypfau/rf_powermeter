#ifndef TEMPSENS_H
#define	TEMPSENS_H


#include <stdbool.h>


void temp_init(void);

void temp_convert(void);
bool temp_done(void);
bool temp_ok(void);
int temp_get_raw_result(void);
long temp_get_result_mdeg(void);


#endif	/* TEMPSENS_H */
