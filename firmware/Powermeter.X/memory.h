#ifndef MEMORY_H
#define	MEMORY_H

#include <stdint.h>
#include <stdbool.h>


#define MEM_I2C_STATUS_OK         0
#define MEM_I2C_STATUS_PENDING   +1
#define MEM_I2C_STATUS_RETRY_ANA +2
#define MEM_I2C_STATUS_RETRY_DNA +3
#define MEM_I2C_ERROR_FAIL       -1
#define MEM_I2C_ERROR_STUCK      -2
#define MEM_I2C_ERROR_LOST       -3
#define MEM_I2C_ERROR_UNKNOWN    -4


void mem_init(void);

bool mem_write(uint16_t address, int size, uint8_t *write_data_buffer);
bool mem_read(uint16_t address, int size, uint8_t *read_data_buffer);
int mem_wait(void);
int mem_check(void);


#endif	/* MEMORY_H */
