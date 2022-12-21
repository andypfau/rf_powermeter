#ifndef RENDER_H
#define	RENDER_H

#include <stdint.h>
#include <stdbool.h>


enum EInput { EInputAvg, EInputFreq, EInputCal, EInputMem };



void render_init(void);

void render_cls(bool remote);
void render_reading(bool remote, int32_t reading);
void render_status(bool remote, bool continuous, int averages, int mhz, bool cal);
void render_help(bool remote);
void render_input(bool remote, enum EInput inputMode, char *buffer, int bufferPos);
void render_error(bool remote, int error);
void render_diag(bool remote, uint32_t vUsb, uint32_t vAnalog, uint32_t temp);
void render_memory(bool remote, int16_t buffer);


#endif	/* RENDER_H */
