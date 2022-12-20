#include "rfsens.h"
#include "infrastructure.h"
#include "mcc_generated_files/spi1.h"


static bool Continuous;
static int Averages;
static int32_t AverageAccu;
static int AverageAccuCount;
static int32_t RawReading;
static int32_t Reading;
static bool NewReading;
static int State;


void rf_init(void)
{
    Continuous = 1;
    Averages = 16;
    AverageAccu = 0;
    AverageAccuCount = 0;
    RawReading = -200000;
    Reading = -200000;
    NewReading = 0;
    State = 0;
    
    infra_set_led(0);
}


void rf_convert(void)
{
    while( SPI1STATLbits.SPITBF == true )
    { /* wait */ }

    SPI1BUFL = 0x0000; // TX buffer; there is not SDO, so this is a dummy
}


bool rf_done(void)
{
    return !(SPI1STATLbits.SPIRBE);
}


int rf_get_raw_result(void)
{
    uint16_t raw = SPI1BUFL;
    
    // strip off the sampling-phase and the parity bits
    int data = ((int)raw >> 1) & 0xFFF;
    
    return data;
}


long rf_convert_to_mdb(long raw, int extra_rsh)
{
    const uint32_t AdcMax = 0xFFFL;
    const float Reference = 3.0;
    const float PostScaling = -39024.39;
    const float PostOffset = 17804.878;
    const int BitShift = 10;
    
    const int32_t Factor = (int32_t)((1L << (BitShift-extra_rsh)) * PostScaling * Reference / AdcMax);
    const int32_t Offset = (int32_t)((1L << BitShift) * PostOffset);

    int32_t result = (int32_t)(((int32_t)(raw) * Factor + Offset) >> BitShift);
    return result;
}


long rf_get_mdb(void)
{
    return rf_convert_to_mdb(rf_get_raw_result(), 0);
}


void rf_fsm_loop(void)
{
    switch (State) {
        case 0:
            rf_convert();
            State = 1;
            break;
        case 1:
            if (rf_done())
            {
                long raw = rf_get_raw_result();
                
                RawReading = rf_convert_to_mdb(raw, 0);
                
                if (RawReading < -50000)
                    infra_set_led(IS_LED_R|IS_LED_G|IS_LED_B);
                else if (RawReading > +10000)
                    infra_set_led(IS_LED_R);
                else
                    infra_set_led(IS_LED_G);
                
                if (AverageAccuCount < Averages) {
                    
                    AverageAccu += raw;
                    AverageAccuCount++;
                    
                    if (AverageAccuCount == Averages)
                        State = 2;
                    else
                        State = 0;
                    
                } else
                    State = 0;
            }
            break;
            
        case 2:
            
            if (AverageAccuCount < 1) {
                // error...
                State = 0;
                break;
            }
            
            int rsh = 0;
            while (AverageAccuCount != 1) {
                AverageAccuCount >>= 1;
                rsh++;
            }
            
            Reading = rf_convert_to_mdb(AverageAccu, rsh);
            
            NewReading = 1;
            AverageAccu = 0;
            
            if (Continuous) {
                AverageAccuCount = 0; // re-start
                State = 0;
            }
            else
                State = 3;
            
            break;
        
        case 3:
        default:
            // stall
            State = 3;
            break;
    }
}


void rf_fsm_trigger(void)
{
    NewReading = 0;
    Continuous = 0;
    AverageAccu = 0;
    AverageAccuCount = 0;
    State = 0; // re-start
}


void rf_fsm_run(void)
{
    NewReading = 0;
    Continuous = 1;
    AverageAccu = 0;
    AverageAccuCount = 0;
    State = 0; // re-start
}


void rf_fsm_stop(void)
{
    NewReading = 0;
    State = 3; // stall
}


void rf_fsm_set_avg(int n)
{
    if (n < 1)
        return;
    NewReading = 0;
    Averages = n;
    AverageAccu = 0;
    AverageAccuCount = 0; // re-start
}


bool rf_fsm_get_mdb(long *reading)
{
    if (!NewReading)
        return 0;

    *reading = Reading;
    NewReading = 0;
    return 1;
}


long rf_fsm_get_mdb_async(void)
{
    return RawReading;
}
