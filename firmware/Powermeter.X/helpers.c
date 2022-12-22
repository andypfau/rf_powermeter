#include "helpers.h"

#include <stdbool.h>


int int_to_str(long value, char* buffer_11b)
{
    int32_t part, remaining, divider, charCount;
    bool started;
    
    charCount = 0;
    started = 0;
    
    if (value < 0) {
        buffer_11b[charCount++] = '-';
        remaining = -value;
    }
    else
        remaining = value;
    
    divider = 1000000000L;
    
    for (int i = 0; i < 10; i++) {
        part = remaining / divider;
        remaining -= part * divider;
        divider /= 10;
        
        if (part != 0)
            started = 1;
        if (i == 9)
            started = 1;
        
        if (started)
            buffer_11b[charCount++] = '0' + part;
    }
    return charCount;
}


int int_to_hex(uint32_t value, char n_digits, char* buffer_8b)
{
    for (int i = 0; i < n_digits; i++) {
        uint8_t digit = (value >> (4*(n_digits-i-1))) & 0xF;
        if (digit <= 9)
            buffer_8b[i] = '0' + digit;
        else
            buffer_8b[i] = 'A' + (digit - 10);
    }
    return n_digits;
}


int fixed_to_str(long value, int exponent, char* buffer_12b)
{
    int32_t part, remaining, divider, charCount, currentExponent;
    bool started;
    
    charCount = 0;
    started = 0;
    
    if (value < 0) {
        buffer_12b[charCount++] = '-';
        remaining = -value;
    }
    else
        remaining = value;
    
    divider = 1000000000L;
    
    for (int i = 0; i < 10; i++) {
        part = remaining / divider;
        remaining -= part * divider;
        divider /= 10;
        
        if (part != 0)
            started = 1;
        if (i == 9)
            started = 1;
        
        if (started)
            buffer_12b[charCount++] = '0' + part;
       
        currentExponent = 9 - i;
        if ((exponent != 0) && (exponent == -currentExponent)) {
            buffer_12b[charCount++] = '.';
            started = 1;
        }
    }
    return charCount;
}


bool parse_int(char* buffer, int len, int *result)
{
    if (len == 0)
        return 0;
    
    *result = 0;
    for (int i = 0; i < len; i++)
    {
        if ((buffer[i] < '0') || (buffer[i] > '9'))
            return 0;
        *result *= 10;
        *result += buffer[i] - '0';
    }
    return 1;
}


bool parse_hex(char* buffer, int len, uint32_t *result)
{
    if (len == 0)
        return 0;
    
    *result = 0;
    for (int i = 0; i < len; i++)
    {
        char value;
        if ((buffer[i] >= '0') && (buffer[i] <= '9'))
            value = buffer[i] - '0';
        else if ((buffer[i] >= 'a') && (buffer[i] <= 'f'))
            value = buffer[i] - 'a' + 0xA;
        else if ((buffer[i] >= 'A') && (buffer[i] <= 'F'))
            value = buffer[i] - 'A' + 0xA;
        else
            return 0;
        *result <<= 4;
        *result += value;
    }
    return 1;
}


int vt100_cls(char* buffer_5b)
{
    buffer_5b[0] = 27; // escape
    buffer_5b[1] = '[';
    buffer_5b[2] = '2';
    buffer_5b[3] = 'J';
    return 4;
}


int vt100_home(char* buffer_5b)
{
    buffer_5b[0] = 27; // escape
    buffer_5b[1] = '[';
    buffer_5b[2] = 'f';
    return 3;
}


int vt100_show_cursor(bool show, char* buffer_6b)
{
    buffer_6b[0] = 27; // escape
    buffer_6b[1] = '[';
    buffer_6b[2] = '?';
    buffer_6b[3] = '2';
    buffer_6b[4] = '5';
    buffer_6b[5] = (show) ? 'h': 'l';
    return 6;
}


