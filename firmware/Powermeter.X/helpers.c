#include "helpers.h"

#include <stdbool.h>


void uint8_to_hex(uint8_t value, char* buffer_2b)
{
    uint8_t part, remaining;
    remaining = value;
    for (int i = 0; i < 2; i++) {
        part = (remaining >> 4) & 0xFF;
        remaining <<= 4;
        if (part > 9)
            buffer_2b[i] = 'A' + part - 10;
        else
            buffer_2b[i] = '0' + part;
    }
}


void uint16_to_hex(uint16_t value, char* buffer_4b)
{
    uint16_t part, remaining;
    remaining = value;
    for (int i = 0; i < 4; i++) {
        part = (remaining >> (3*4)) & 0xFF;
        remaining <<= 4;
        if (part > 9)
            buffer_4b[i] = 'A' + part - 10;
        else
            buffer_4b[i] = '0' + part;
    }
}


void uint32_to_hex(uint32_t value, char* buffer_8b)
{
    uint32_t part, remaining;
    remaining = value;
    for (int i = 0; i < 8; i++) {
        part = (remaining >> (7*4)) & 0xFF;
        remaining <<= 4;
        if (part > 9)
            buffer_8b[i] = 'A' + part - 10;
        else
            buffer_8b[i] = '0' + part;
    }
}


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
        
        if (started)
            buffer_11b[charCount++] = '0' + part;
    }
    return charCount;
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


#define N_CHARS 14
static const char* LINE_0 = " #  #  # ## #  ### ##### #  #             ";
static const char* LINE_1 = "# ### # #  ##  #  #    ## ## #    #       ";
static const char* LINE_2 = "# # #   # ####### ##   # #   #   ######   ";
static const char* LINE_3 = "# # #  #   # #   ## # # # #  #    #       ";
static const char* LINE_4 = " # ########  # ##  #  #  # ##  #          ";
static const char* CHARS = "0123456789.+- ";


int big_str(char* in_buffer, int in_buffer_size, int line, char* out_buffer, int out_buffer_size)
{
    char* refBuf;
    int pos;
    
    switch (line) {
        case 0: refBuf = (char*)LINE_0; break;
        case 1: refBuf = (char*)LINE_1; break;
        case 2: refBuf = (char*)LINE_2; break;
        case 3: refBuf = (char*)LINE_3; break;
        case 4: refBuf = (char*)LINE_4; break;
        default: return 0;
    }
    
    pos = 0;
    for (int i = 0; i < in_buffer_size; i++)
    {
        if (i > 0) {
            if (pos+2 >= out_buffer_size)
                return pos;
            out_buffer[pos++] = ' ';
            out_buffer[pos++] = ' ';
        }
        
        if (pos+3 >= out_buffer_size)
            return pos;
        
        for (int j = 0; j < N_CHARS; j++) {
            if (in_buffer[i] == CHARS[j]) {
                out_buffer[pos++] = refBuf[3*j+0];
                out_buffer[pos++] = refBuf[3*j+1];
                out_buffer[pos++] = refBuf[3*j+2];
                break;
            }
        }
    }
    return pos;
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


bool parse_mega(char* buffer, int len, int *result)
{
    if (len == 0)
        return 0;
    
    int shift;
    switch (buffer[len-1]) {
        case 'm':
            shift = 0;
            break;
        case 'g':
            shift = 3;
            break;
        default:
            return 0;
    }
    
    *result = 0;
    for (int i = 0; i < len-1; i++)
    {
        if ((buffer[i] < '0') || (buffer[i] > '9'))
            return 0;
        *result *= 10;
        *result += buffer[i] - '0';
    }
    for (int i = 0; i < shift; i++)
        *result *= 10;
    return 1;
}
