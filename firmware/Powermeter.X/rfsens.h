#ifndef RFSENS_H
#define	RFSENS_H

#include <stdint.h>
#include <stdbool.h>


void rf_init(void);
void rf_loop(void);

bool rf_waiting(void);
bool rf_continuous(void);
int rf_get_avg(void);

void rf_trigger(void);
void rf_run(void);
void rf_stop(void);
void rf_resume(void);
void rf_set_avg(int n);
bool rf_get_mdb(long *reading);


#endif	/* RFSENS_H */
