#ifndef HELPERS_H
#define	HELPERS_H


#include <stdint.h>
#include <stdbool.h>


#define IS_POWER_OF_2(x) ((x & (x - 1)) == 0)


void uint8_to_hex(uint8_t value, char* buffer_2b);
void uint16_to_hex(uint16_t value, char* buffer_4b);
void uint32_to_hex(uint32_t value, char* buffer_8b);

int int_to_str(long value, char* buffer_11b);
int fixed_to_str(long value, int exponent, char* buffer_12b);

int big_str(char* in_buffer, int in_buffer_size, int line, char* out_buffer, int out_buffer_size);

int vt100_cls(char* buffer_5b);
int vt100_home(char* buffer_5b);
int vt100_show_cursor(bool show, char* buffer_6b);

bool parse_int(char* buffer, int len, int *result);


#endif // HELPERS_H
