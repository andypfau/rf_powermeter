#include "render.h"
#include "helpers.h"
#include "usb.h"
#include "rfsens.h"

#include <string.h>


#define STR_BUF_SIZE 64

static char StrBuffer[STR_BUF_SIZE];


void render_init(void)
{
}


void render_cls(bool remote)
{
    if (remote)
        return;
    
    int len = 0;
    
    len += vt100_show_cursor(0, &(StrBuffer[len]));
    len += vt100_cls(&(StrBuffer[len]));
    len += vt100_home(&(StrBuffer[len]));
    
    memcpy(&(StrBuffer[len]), "\n\n\n|", 4);
    len += 4;

    for(int i = 0; i < 7; i++) {
        memcpy(&(StrBuffer[len]), " ' |", 4);
        len += 4;
    }
    
    usb_set_data(StrBuffer, len);
}


void render_reading(bool remote, int32_t reading)
{
    int len = 0;
    
    if (remote) {
        
        len += fixed_to_str(reading, -3, 3, &(StrBuffer[len]));
        StrBuffer[len++] = '\n';
        
    } else {
        
        len += vt100_home(&(StrBuffer[len]));

        int digits = (rf_get_avg() > 100) ? 3 : 2;
        len += fixed_to_str(reading, -3, digits, &(StrBuffer[len]));
        memcpy(&(StrBuffer[len]), " dBm  \r\n\n", 9);
        len += 9;

        int pos = (60000 + reading) / 2500;
        if (pos <  0) pos =  0;
        if (pos > 28) pos = 28;
        for(int i = 0; i < pos; i++)
            StrBuffer[len++] = ' ';
        StrBuffer[len++] = 'v';
        for(int i = 0; i < 28-pos; i++)
            StrBuffer[len++] = ' ';
    }

    usb_set_data(StrBuffer, len);
}


void render_status(bool remote, bool continuous, int averages, int mhz, bool cal)
{
    if (remote)
        return;        
    
    int len = 0;
    
    memcpy(&(StrBuffer[len]), "\r\n\n\n", 4);
    len += 4;

    if (continuous)
        memcpy(&(StrBuffer[len]), "Cnt", 3);
    else
        memcpy(&(StrBuffer[len]), "Trg", 3);
    len += 3;
    
    memcpy(&(StrBuffer[len]), "  Avg ", 6);
    len += 6;
    len += int_to_str(averages, &(StrBuffer[len]));
    
    memcpy(&(StrBuffer[len]), "  ", 2);
    len += 2;
    len += int_to_str(mhz, &(StrBuffer[len]));
    memcpy(&(StrBuffer[len]), " MHz", 4);
    len += 4;
    
    if (!cal) {
        memcpy(&(StrBuffer[len]), "  Unc", 5);
        len += 5;
    }
    
    usb_set_data(StrBuffer, len);
}


void render_help(bool remote)
{
    if (remote)
        return;        
    
    int len = 0;
    
    len += vt100_cls(&(StrBuffer[len]));
    len += vt100_home(&(StrBuffer[len]));
    
    memcpy(&(StrBuffer[len]), "C|T: Cont / Trig\r\n", 18);
    len += 18;
    memcpy(&(StrBuffer[len]), "A|F: Avg/Freq\r\n", 15);
    len += 15;
    memcpy(&(StrBuffer[len]), "D:   Diag\r\n", 11);
    len += 11;
    memcpy(&(StrBuffer[len]), "Esc: Redraw\r\n", 13);
    len += 13;
    
    usb_set_data(StrBuffer, len);
}


void render_input(bool remote, enum EInput inputMode, char *buffer, int bufferPos)
{
    if (remote)
        return;        
    
    int len = 0;
    
    len += vt100_cls(&(StrBuffer[len]));
    len += vt100_home(&(StrBuffer[len]));
    
    if (inputMode == EInputAvg) {
        memcpy(&(StrBuffer[len]), "Avg (1..512): ", 15);
        len += 15;
    } else if (inputMode == EInputFreq) {
        memcpy(&(StrBuffer[len]), "MHz (10..8000): ", 16);
        len += 16;
    } else if (inputMode == EInputMem) {
        // only implemented for remote
    } else if (inputMode == EInputCal) {
        memcpy(&(StrBuffer[len]), "Cal (0/1): ", 11);
        len += 11;
    }
    
    memcpy(&(StrBuffer[len]), buffer, bufferPos);
    len += bufferPos;
    
    usb_set_data(StrBuffer, len);
}


void render_error(bool remote, int error)
{
    int len = 0;
    
    len += int_to_str(error, &(StrBuffer[len]));
    StrBuffer[len++] = '\n';
    
    usb_set_data(StrBuffer, len);
}


void render_diag(bool remote, uint32_t vUsb, uint32_t vAnalog, uint32_t temp)
{
    int len = 0;
    
    if (remote) {
        
        len += fixed_to_str(vUsb, -3, 3, &(StrBuffer[len]));
        StrBuffer[len++] = ';';
        len += fixed_to_str(vAnalog, -3, 3, &(StrBuffer[len]));
        StrBuffer[len++] = ';';
        len += fixed_to_str(temp, -3, 3, &(StrBuffer[len]));
        StrBuffer[len++] = '\n';
        
    } else {
        len += vt100_cls(&(StrBuffer[len]));
        len += vt100_home(&(StrBuffer[len]));

        memcpy(&(StrBuffer[len]), "USB:  ", 6);
        len += 6;
        len += fixed_to_str(vUsb, -3, 2, &(StrBuffer[len]));

        memcpy(&(StrBuffer[len]), "\r\n5V0A: ", 8);
        len += 8;
        len += fixed_to_str(vAnalog, -3, 2, &(StrBuffer[len]));

        memcpy(&(StrBuffer[len]), "\r\nTemp: ", 8);
        len += 8;
        len += fixed_to_str(temp, -3, 1, &(StrBuffer[len]));
    }
    
    usb_set_data(StrBuffer, len);
}


void render_memory(bool remote, int16_t buffer)
{
    if (!remote)
        return;
    
    int len = 0;
    
    len += int_to_hex(buffer, 4, &(StrBuffer[len]));
    StrBuffer[len++] = '\n';
    
    usb_set_data(StrBuffer, len);
}
