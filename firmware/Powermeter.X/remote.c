#include "remote.h"
#include "usb.h"
#include "infrastructure.h"
#include "mcc_generated_files/tmr1.h"
#include "tempsens.h"
#include "rfsens.h"
#include "memory.h"
#include "helpers.h"

#include <stdio.h>
#include <string.h>


#define STR_BUF_SIZE 64



static char StrBuffer[STR_BUF_SIZE];


void remote_init(void)
{
}


void remote_enter(void)
{
    rf_fsm_stop();
    rf_fsm_set_avg(512);
}


bool remote_loop(void)
{
    if (!usb_ready_to_send())
        return 1;
    
    long reading;
    if (rf_fsm_get_mdb(&reading))
    {
        int len = fixed_to_str(reading, -3, StrBuffer);
        StrBuffer[len++] = '\n';
        usb_set_data(StrBuffer, len);
    }
    
    char usbCmd = 0;
    if (usb_get_data(&usbCmd, 1)) {
        switch(usbCmd) {
            case 't':
                rf_fsm_trigger();
                break;
            case 27: // escape
                return 0; // leave remote-mode
        }
    }
    
    return 1;
}
