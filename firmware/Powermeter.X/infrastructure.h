#ifndef INFRASTRUCTURE_H
#define	INFRASTRUCTURE_H


#include <stdbool.h>
#include "mcc_generated_files/adc1.h"


void infra_init(void);


bool infra_get_button(void);


#define IS_LED_R 1
#define IS_LED_G 2
#define IS_LED_B 4

void infra_set_led(int leds);
int infra_get_led(void);


void infra_enable_supply(bool enable);
void infra_enable_rf(bool enable);


#define IS_ADC_5V0USB_MILLIVOLTS Channel_S5V0USB
#define IS_ADC_5V0A_MILLIVOLTS   Channel_S5V0A
// note that the RF sensor de-soldered in PCB rev.2, due to HW-bug


void infra_adc_sample(int channel);
void infra_adc_convert(void);
bool infra_adc_done(void);
int infra_adc_get_raw_result(void);
int infra_adc_get_result(void);


bool infra_acquire_i2c(void);
void infra_release_i2c(void);


#endif	/* INFRASTRUCTURE_H */
