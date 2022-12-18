#ifndef MEMORY_H
#define	MEMORY_H

#include <stdint.h>
#include <stdbool.h>


void mem_init(void);

void mem_write(int address, int size, uint8_t *buffer);
void mem_read(int address, int size, uint8_t *buffer);
void mem_wait(void);
bool mem_done(void);


#endif	/* MEMORY_H */
