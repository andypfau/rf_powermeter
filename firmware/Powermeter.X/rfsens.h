#ifndef RFSENS_H
#define	RFSENS_H

#include <stdint.h>
#include <stdbool.h>


void rf_init(void);

void rf_convert(void);
bool rf_done(void);
int rf_get_raw_result(void);
long rf_convert_to_mdb(long raw, int extra_rsh);
long rf_get_mdb(void);

void rf_fsm_loop(void);
void rf_fsm_trigger(void);
void rf_fsm_run(void);
void rf_fsm_stop(void);
void rf_fsm_set_avg(int n);
bool rf_fsm_get_mdb(long *reading);
long rf_fsm_get_mdb_async(void);


#endif	/* RFSENS_H */
