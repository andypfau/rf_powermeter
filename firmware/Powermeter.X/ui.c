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
#include "render.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#define IN_BUF_SIZE  10


#define ERROR_INVALID_INPUT    11
#define ERROR_TEMP_READ_FAILED 21
#define ERROR_I2C_BUSY         22


static char InBuffer[IN_BUF_SIZE];
static int InBufferPos;

static int LastTickCount;

static bool ApplyCal;
static bool Remote;

static long Reading;
static int DiagVUsb, DiagVA;
static long DiagTemp;
static uint8_t Error;

static enum { Idle, Redraw, Redraw2, DispReading, DispStatus, InputAvg, InputFreq, InputMem, InputCal, DispHelp, DispDiag, DispError } State;
static char DiagFsmState;


void ui_init(void)
{
    infra_set_led(0);
    
    InBufferPos = 0;
    ApplyCal = 0;
    Reading = 0;
    State = Idle;
    DiagFsmState = 0;
    DiagVA = 0;
    DiagVUsb = 0;
    DiagTemp = 0;
    Remote = 1;
    Error = 0;
    
    cal_load(10);
    
    rf_stop();
    rf_set_avg(16);
    
    TMR1_Period16BitSet(2500); // 100 Hz
    TMR1_Start();
    LastTickCount = TMR1_SoftwareCounterGet();
}


void ui_loop(void)
{
    if (!usb_ready_to_send())
        return;
    
    int tickCount = TMR1_SoftwareCounterGet();
    bool tick = 0;
    if (tickCount != LastTickCount) {
        LastTickCount = tickCount;
        tick = 1;
    }
    
    switch (State) {
        case Idle:
            if (rf_get_mdb(&Reading)) {
                if (ApplyCal)
                    Reading = cal_apply(Reading);
                State = DispReading;
            }
            break;
        
        case Redraw:
            if (Remote) {
                State = Idle;
            } else {
                render_cls(Remote);
                State = Redraw2;
            }
            break;
            
        case Redraw2:
            if (Remote) {
                State = Idle;
            } else {
                render_reading(Remote, Reading);
                State = DispStatus;
            }
            break;
            
        case DispStatus:
            render_status(Remote, rf_continuous(), rf_get_avg(), cal_get_mhz(), ApplyCal);
            State = Idle;
            break;
        
        case DispHelp:
            render_help(Remote);
            State = Idle;
            break;
        
        case DispReading:
            render_reading(Remote, Reading);
            State = Idle;
            break;
        
        case DispDiag:
            switch (DiagFsmState) {
                case 0:
                    if (tick) {
                        infra_adc_sample(IS_ADC_5V0A_MILLIVOLTS);
                        DiagFsmState = 1;
                    }
                    break;
                case 1:
                    if (tick) {
                        infra_adc_convert();
                        DiagFsmState = 2;
                    }
                    break;
                case 2:
                    if (infra_adc_done()) {
                        DiagVA = infra_adc_get_result();
                        infra_adc_sample(IS_ADC_5V0USB_MILLIVOLTS);
                        DiagFsmState = 3;
                    }
                    break;
                case 3:
                    if (tick) {
                        infra_adc_convert();
                        DiagFsmState = 4;
                    }
                    break;
                case 4:
                    if (infra_adc_done()) {
                        DiagVUsb = infra_adc_get_result();
                        DiagFsmState = 5;
                    }
                    break;
                case 5:
                    if (infra_acquire_i2c()) {
                        temp_convert();
                        DiagFsmState = 6;
                    }
                    break;
                case 6:
                    if (temp_done()) {
                        infra_release_i2c();
                        if (temp_ok())
                            DiagTemp = temp_get_result_mdeg();
                        else {
                            Error = ERROR_TEMP_READ_FAILED;
                            DiagTemp = 0;
                        }
                        render_diag(Remote, DiagVUsb, DiagVA, DiagTemp);
                        
                        if (Remote)
                            State = Idle;
                        else
                            DiagFsmState = 0; // repeat diag
                    }
                    break;
                default:
                    DiagFsmState = 0;
                    break;
            }
            
            break;
        
        case InputAvg:
            render_input(Remote, EInputAvg, InBuffer, InBufferPos);
            break;

        case InputFreq:
            render_input(Remote, EInputFreq, InBuffer, InBufferPos);
            break;

        case InputMem:
            render_input(Remote, EInputMem, InBuffer, InBufferPos);
            break;

        case InputCal:
            render_input(Remote, EInputCal, InBuffer, InBufferPos);
            break;

        case DispError:
            render_error(Remote, Error);
            Error = 0;
            State = Idle;
            break;
    }
    
    char usbCmd = 0;
    if (!usb_get_data(&usbCmd, 1))
        return;
    
    switch (State) {
        case Idle:
            switch (usbCmd) {
                case 27: // escape
                    Remote = 0;
                    rf_run();
                    State = Redraw;
                    break;
                case 'a':
                    InBufferPos = 0;
                    State = InputAvg;
                    break;
                case 'c':
                    if (!Remote) {
                        rf_run();
                        State = DispStatus;
                    }
                    break;
                case 'd':
                    State = DispDiag;
                    DiagFsmState = 0;
                    break;
                case 'e':
                    if (Remote)
                        State = DispError;
                    break;
                case 't':
                    rf_trigger();
                    State = DispStatus;
                    break;
                case 'f':
                    InBufferPos = 0;
                    State = InputFreq;
                    break;
                case 'h':
                    if (!Remote)
                        State = DispHelp;
                    break;
                case 'm':
                    InBufferPos = 0;
                    State = InputCal;
                    break;
                case 0:
                case 'r': // for debugging only
                    rf_stop();
                    Remote = 1;
                    Error = 0;
                    State = Idle;
                    break;
                case 'w':
                    if (Remote) {
                        InBufferPos = 0;
                        State = InputMem;
                    }
                    break;
                default:
                    // ignore
                    break;
            }
            break;
            
        case DispHelp:
        case DispDiag:
            if (usbCmd == 27) {
                State = Redraw;
            }
            break;
            
        case InputAvg:
        case InputFreq:
        case InputMem:
        case InputCal:
            if (usbCmd == '\n' || usbCmd == '\r') {
                if (State == InputAvg) {
                    int n;
                    if (parse_int(InBuffer, InBufferPos, &n)) {
                        if ((IS_POWER_OF_2(n)) && (n >= 1) && (n <= 512)) {
                            rf_set_avg(n);
                        } else {
                            // not a power of 2, or out of range
                            Error = ERROR_INVALID_INPUT;
                        }
                    } else
                        Error = ERROR_INVALID_INPUT;
                } else if (State == InputFreq) {
                    int tmp;
                    if (parse_int(InBuffer, InBufferPos, &tmp)) {
                        if ((tmp >= 10) && (tmp <= 8000)) {
                            cal_load(tmp);
                        } else
                            Error = ERROR_INVALID_INPUT;
                    } else
                        Error = ERROR_INVALID_INPUT;
                } else if (State == InputMem) {
                    if (InBufferPos == 8) {
                        uint32_t tmp;
                        if (parse_hex(InBuffer, InBufferPos, &tmp)) {
                            uint16_t address = (tmp >> 16) & 0xFFFF;
                            uint16_t buffer = tmp & 0xFFFF;
                            if (infra_acquire_i2c()) {
                                mem_write(address, 2, (uint8_t*)(&buffer));
                                mem_wait();
                                infra_release_i2c();
                            } else
                                Error = ERROR_I2C_BUSY;
                        } else
                            Error = ERROR_INVALID_INPUT;
                    }
                } else if (State == InputCal) {
                    if (InBufferPos == 1) {
                        if (InBuffer[0] == '0')
                            ApplyCal = 0;
                        else if (InBuffer[0] == '1')
                            ApplyCal = 1;
                        else
                            Error = ERROR_INVALID_INPUT;
                    } else
                        Error = ERROR_INVALID_INPUT;
                }
                State = Redraw;
            } else if (InBufferPos < IN_BUF_SIZE) {
                InBuffer[InBufferPos] = usbCmd;
                InBufferPos++;
            } else { // overflow
                Error = ERROR_INVALID_INPUT;
                State = Redraw;
            }
            break;
        
        default:
            break;
    }
        
    return;
}
