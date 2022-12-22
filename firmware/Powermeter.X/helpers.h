#ifndef HELPERS_H
#define	HELPERS_H


#include <stdint.h>
#include <stdbool.h>


#define IS_POWER_OF_2(x) ((x & (x - 1)) == 0)


int int_to_str(long value, char* buffer_11b);
int int_to_hex(uint32_t value, char n_digits, char* buffer_8b);
int fixed_to_str(long value, int exponent, char* buffer_12b);

bool parse_int(char* buffer, int len, int *result);
bool parse_hex(char* buffer, int len, uint32_t *result);

int vt100_cls(char* buffer_5b);
int vt100_home(char* buffer_5b);
int vt100_show_cursor(bool show, char* buffer_6b);


#endif // HELPERS_H
