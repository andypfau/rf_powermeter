#include "mcc_generated_files/system.h"
#include "mcc_generated_files/interrupt_manager.h"

#include "infrastructure.h"
#include "tempsens.h"
#include "rfsens.h"
#include "memory.h"
#include "ui.h"
#include "usb.h"
#include "cal.h"

#include <stdbool.h>


int main(void)
{
    SYSTEM_Initialize();
    INTERRUPT_GlobalEnable();
    
    infra_init();
    temp_init();
    rf_init();
    mem_init();
    cal_init();
    
    infra_enable_supply(1);
    infra_enable_rf(1);
    
    usb_init();
    ui_init();
    
    while (1)
    {
        rf_fsm_loop();
        ui_loop();
        usb_loop();
    }

    return 1;
}
