#include "ui.h"
#include "usb.h"
#include "infrastructure.h"
#include "mcc_generated_files/tmr1.h"
#include "tempsens.h"
#include "rfsens.h"
#include "memory.h"
#include "cal.h"
#include "helpers.h"
#include "memory.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


enum UiModeT{ UiModeMain, UiModeDiag, UiModeRemote };


#define MEM_BUF_SIZE 16
#define STR_BUF_SIZE 64
#define IN_BUF_SIZE 5


static char StrBuffer[STR_BUF_SIZE];
static char InBuffer[IN_BUF_SIZE];
static int InBufferPos;
static bool Continuous;
static int Averages;
static bool AnyUpdate;
static bool WaitingForData;
static bool RedrawNeeded;
static long Reading;
static int DiagVUsb, DiagVA;
static long DiagTemp;
static char DiagState;
static int MHz;
static bool ApplyCal;
static enum { DispNormal, InputAvg, InputFreq, InputMem, InputCal, DispHelp, DispDiag } CurrentMode;
static int Line;
static int LastCount;
static bool Remote;


void cls(void)
{
    if (Remote)
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
    RedrawNeeded = 0;
}


void render_reading(void)
{
    int len = 0;
    
    if (Remote) {
        
        len += fixed_to_str(Reading, -3, &(StrBuffer[len]));
        StrBuffer[len++] = '\n';
        
    } else {
        
        len += vt100_home(&(StrBuffer[len]));

        len += fixed_to_str(Reading, -3, &(StrBuffer[len]));
        memcpy(&(StrBuffer[len]), " dB  \r\n\n", 8);
        len += 8;

        int pos = (60000 + Reading) / 2500;
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


void render_status(void)
{
    if (Remote)
        return;        
    
    int len = 0;
    
    memcpy(&(StrBuffer[len]), "\r\n\n\n", 4);
    len += 4;

    if (Continuous)
        memcpy(&(StrBuffer[len]), "Cnt", 3);
    else if (WaitingForData)
        memcpy(&(StrBuffer[len]), "Trg", 3);
    else
        memcpy(&(StrBuffer[len]), "Stp", 3);
    len += 3;
    
    memcpy(&(StrBuffer[len]), "  Avg ", 6);
    len += 6;
    len += int_to_str(Averages, &(StrBuffer[len]));
    
    memcpy(&(StrBuffer[len]), "  ", 2);
    len += 2;
    len += int_to_str(MHz, &(StrBuffer[len]));
    memcpy(&(StrBuffer[len]), " MHz", 4);
    len += 4;
    
    if (!ApplyCal) {
        memcpy(&(StrBuffer[len]), "  Unc", 5);
        len += 5;
    }
    
    usb_set_data(StrBuffer, len);
}


void render_help(void)
{
    if (Remote)
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


void render_input(void)
{
    if (Remote)
        return;        
    
    int len = 0;
    
    len += vt100_cls(&(StrBuffer[len]));
    len += vt100_home(&(StrBuffer[len]));
    
    if (CurrentMode == InputAvg) {
        memcpy(&(StrBuffer[len]), "Avg (1..512): ", 15);
        len += 15;
    } else if (CurrentMode == InputFreq) {
        memcpy(&(StrBuffer[len]), "MHz (10..8000): ", 16);
        len += 16;
    } else if (CurrentMode == InputMem) {
        // only implemented for remote
    } else if (CurrentMode == InputCal) {
        memcpy(&(StrBuffer[len]), "Cal (0/1): ", 11);
        len += 11;
    }
    
    memcpy(&(StrBuffer[len]), InBuffer, InBufferPos);
    len += InBufferPos;
    
    usb_set_data(StrBuffer, len);
}


void render_diag(void)
{
    int len = 0;
    
    if (Remote) {
        
        len += fixed_to_str(DiagVUsb, -3, &(StrBuffer[len]));
        StrBuffer[len++] = ';';
        len += fixed_to_str(DiagVA, -3, &(StrBuffer[len]));
        StrBuffer[len++] = ';';
        len += fixed_to_str(DiagTemp, -3, &(StrBuffer[len]));
        StrBuffer[len++] = '\n';
        
    } else {
        len += vt100_cls(&(StrBuffer[len]));
        len += vt100_home(&(StrBuffer[len]));

        memcpy(&(StrBuffer[len]), "USB:  ", 6);
        len += 6;
        len += fixed_to_str(DiagVUsb, -3, &(StrBuffer[len]));

        memcpy(&(StrBuffer[len]), "\r\n5V0A: ", 8);
        len += 8;
        len += fixed_to_str(DiagVA, -3, &(StrBuffer[len]));

        memcpy(&(StrBuffer[len]), "\r\nTemp: ", 8);
        len += 8;
        len += fixed_to_str(DiagTemp, -3, &(StrBuffer[len]));
    }
    
    usb_set_data(StrBuffer, len);
}


void schedule_redraw(bool complete)
{
    if (complete)
        RedrawNeeded = 1;
    AnyUpdate = 1;
    Line = 0;
}


void abort_scheduled_redraw()
{
    RedrawNeeded = 0;
    AnyUpdate = 0;
    Line = 0;
}


void ui_init(void)
{
    infra_set_led(0);
    
    InBufferPos = 0;
    Continuous = 1;
    Averages = 16;
    AnyUpdate = 0;
    ApplyCal = 1;
    WaitingForData = 1;
    RedrawNeeded = 0;
    Reading = 0;
    MHz = 1000;
    CurrentMode = DispNormal;
    Line = 0;
    DiagState = 0;
    DiagVA = 0;
    DiagVUsb = 0;
    DiagTemp = 0;
    Remote = 1;
    
    cal_load(MHz);
    
    rf_fsm_run();
    rf_fsm_set_avg(Averages);
    schedule_redraw(1);
    
    TMR1_Period16BitSet(2500); // 100 Hz
    TMR1_Start();
    LastCount = TMR1_SoftwareCounterGet();
}


void ui_loop(void)
{
    if (!usb_ready_to_send())
        return;
    
    int tickCount = TMR1_SoftwareCounterGet();
    bool tick = 0;
    if (tickCount != LastCount) {
        LastCount = tickCount;
        tick = 1;
    }
    
    switch (CurrentMode) {
        case DispNormal:
            if (rf_fsm_get_mdb(&Reading)) {
                if (ApplyCal) {
                    Reading = cal_apply(Reading);
                }
                WaitingForData = 0;
                schedule_redraw(0);
            }
            if (RedrawNeeded) {
                cls();
                schedule_redraw(0);
            } else if (AnyUpdate) {
                switch (Line) {
                    case 0:
                        render_reading();
                        Line = 1;
                        break;
                    case 1:
                        render_status();
                        Line = 0;
                        AnyUpdate = 0;
                        break;
                    default:
                        Line = 0;
                        break;
                }
            }
            break;
        
        case DispHelp:
            if (AnyUpdate) {
                render_help();
                AnyUpdate = 0;
            }  
            break;
        
        case DispDiag:
            if (AnyUpdate) {
                render_diag();
                AnyUpdate = 0;
                if (Remote)
                    CurrentMode = DispNormal;
            }
            
            switch (DiagState) {
                case 0:
                    if (tick) {
                        infra_adc_sample(IS_ADC_5V0A_MILLIVOLTS);
                        DiagState = 1;
                    }
                    break;
                case 1:
                    if (tick) {
                        infra_adc_convert();
                        DiagState = 2;
                    }
                    break;
                case 2:
                    if (infra_adc_done()) {
                        DiagVA = infra_adc_get_result();
                        infra_adc_sample(IS_ADC_5V0USB_MILLIVOLTS);
                        DiagState = 3;
                    }
                    break;
                case 3:
                    if (tick) {
                        infra_adc_convert();
                        DiagState = 4;
                    }
                    break;
                case 4:
                    if (infra_adc_done()) {
                        DiagVUsb = infra_adc_get_result();
                        DiagState = 5;
                    }
                    break;
                case 5:
                    if (infra_acquire_i2c()) {
                        temp_convert();
                        DiagState = 6;
                    }
                    break;
                case 6:
                    if (temp_done()) {
                        infra_release_i2c();
                        if (temp_ok())
                            DiagTemp = temp_get_result_mdeg();
                        else
                            DiagTemp = 0;
                        schedule_redraw(0);
                        
                        if (Remote)
                            DiagState = 7;
                        else
                            DiagState = 0;
                    }
                    break;
                case 7:
                    break;
                default:
                    DiagState = 0;
                    break;
            }
            
            break;
        
        case InputAvg:
        case InputFreq:
        case InputMem:
        case InputCal:
            if (AnyUpdate) {
                render_input();
                AnyUpdate = 0;
            }  
            break;
    }
    
    char usbCmd = 0;
    if (!usb_get_data(&usbCmd, 1))
        return;
    
    switch (CurrentMode) {
        case DispNormal:
            switch (usbCmd) {
                case 27: // escape
                    Remote = 0;
                    schedule_redraw(1);
                    break;
                case 'c':
                    if (Remote)
                        break;
                    Continuous = 1;
                    rf_fsm_run();
                    schedule_redraw(0);
                    break;
                case 't':
                    Continuous = 0;
                    WaitingForData = 1;
                    rf_fsm_trigger();
                    if (!Remote)
                        schedule_redraw(0);
                    break;
                case 'a':
                    InBufferPos = 0;
                    CurrentMode = InputAvg;
                    if (!Remote)
                        schedule_redraw(0);
                    break;
                case 'f':
                    InBufferPos = 0;
                    CurrentMode = InputFreq;
                    if (!Remote)
                        schedule_redraw(0);
                    break;
                case 'm':
                    InBufferPos = 0;
                    CurrentMode = InputCal;
                    if (!Remote)
                        schedule_redraw(0);
                    break;
                case 'w':
                    if (!Remote)
                        break;
                    InBufferPos = 0;
                    CurrentMode = InputMem;
                    break;
                case 'h':
                    if (Remote)
                        break;
                    CurrentMode = DispHelp;
                    schedule_redraw(0);
                    break;
                case 'd':
                    CurrentMode = DispDiag;
                    DiagState = 0;
                    abort_scheduled_redraw();
                    break;
                case 0:
                case 'r': // for testing only
                    abort_scheduled_redraw();
                    rf_fsm_stop();
                    Remote = 1;
                    break;
                default:
                    // ignore
                    break;
            }
            break;
            
        case DispHelp:
        case DispDiag:
            if (usbCmd == 27) {
                CurrentMode = DispNormal;
                schedule_redraw(1);
            }
            break;
            
        case InputAvg:
        case InputFreq:
        case InputMem:
        case InputCal:
            if (usbCmd == '\n' || usbCmd == '\r') {
                if (CurrentMode == InputAvg) {
                    int n;
                    if (parse_int(InBuffer, InBufferPos, &n)) {
                        if ((IS_POWER_OF_2(n)) && (n >= 1) && (n <= 512)) {
                            Averages = n;
                            rf_fsm_set_avg(n);
                        } else {
                            // not a power of 2, or out of range
                        }
                    }
                } else if (CurrentMode == InputFreq) {
                    int tmp;
                    if (parse_int(InBuffer, InBufferPos, &tmp)) {
                        if ((tmp >= 10) && (tmp <= 8000)) {
                            MHz = tmp;
                            cal_load(MHz);
                        }
                    }
                } else if (CurrentMode == InputMem) {
                    uint32_t tmp;
                    if (InBufferPos == 9) {
                        if (parse_hex(InBuffer, InBufferPos, &tmp)) {
                            int address = (tmp >> 16) & 0xFFFF;
                            uint16_t buffer = tmp & 0xFFFF;
                            if (infra_acquire_i2c()) {
                                mem_write(address, 2, (uint8_t*)(&buffer));
                                mem_wait();
                                infra_release_i2c();
                            }
                        }
                    }
                } else if (CurrentMode == InputCal) {
                    if (InBufferPos == 1) {
                        if (InBuffer[0] == '0')
                            ApplyCal = 0;
                        else if (InBuffer[0] == '1')
                            ApplyCal = 1;
                    }
                }
                CurrentMode = DispNormal;
                schedule_redraw(1);
            } else if (InBufferPos < IN_BUF_SIZE) {
                InBuffer[InBufferPos] = usbCmd;
                InBufferPos++;
                schedule_redraw(0);
            } else { // overflow
                CurrentMode = DispNormal;
                schedule_redraw(1);
            }
            break;
    }
        
    return;
}
