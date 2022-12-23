#include "rfsens.h"
#include "infrastructure.h"
#include "mcc_generated_files/spi1.h"


static bool Continuous;
static int Averages;
static int32_t AverageAccu;
static int AverageAccuCount;
static int32_t RawReading;
static int32_t Reading;
static bool Enable;
static bool NewReading;
static int State;
static bool WaitingForData;


void convert(void)
{
    while( SPI1STATLbits.SPITBF == true )
    { /* wait */ }

    SPI1BUFL = 0x0000; // TX buffer; there is not SDO, so this is a dummy
}


bool done(void)
{
    return !(SPI1STATLbits.SPIRBE);
}


int get_raw_result(void)
{
    uint16_t raw = SPI1BUFL;
    
    // strip off the sampling-phase and the parity bits
    int data = ((int)raw >> 1) & 0xFFF;
    
    return data;
}


long convert_to_mdb(long raw, int extra_rsh)
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


long get_mdb(void)
{
    return convert_to_mdb(get_raw_result(), 0);
}


void rf_init(void)
{
    Continuous = 0;
    Averages = 16;
    AverageAccu = 0;
    AverageAccuCount = 0; // start
    RawReading = 0;
    Reading = 0;
    NewReading = 0;
    Enable = 0;
    State = 0;
    
    infra_set_led(0);
}


void rf_loop(void)
{
    switch (State) {
        case 0:
            convert();
            State = 1;
            break;
        case 1:
            if (done())
            {
                long raw = get_raw_result();
                
                RawReading = convert_to_mdb(raw, 0);
                
                if (RawReading < -50000)
                    infra_set_led(IS_LED_R|IS_LED_G|IS_LED_B);
                else if (RawReading > +10000)
                    infra_set_led(IS_LED_R);
                else
                    infra_set_led(IS_LED_G);
                
                AverageAccu += raw;
                
                AverageAccuCount++;
                if (AverageAccuCount == Averages)
                    State = 2;
                else
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
            
            if (Enable && WaitingForData) {
                Reading = convert_to_mdb(AverageAccu, rsh);
                NewReading = 1;
                WaitingForData = Continuous;
            } else
                WaitingForData = 0;
            
            AverageAccu = 0;
            AverageAccuCount = 0;
            State = 0;
            
            break;
        
        default:
            State = 0;
            break;
    }
}


void rf_trigger(void)
{
    NewReading = 0;
    Continuous = 0;
    AverageAccu = 0;
    AverageAccuCount = 0; // re-start
    Enable = 1;
    WaitingForData = 1;
}


void rf_run(void)
{
    NewReading = 0;
    Continuous = 1;
    AverageAccu = 0;
    AverageAccuCount = 0; // re-start
    Enable = 1;
    WaitingForData = 1;
}


void rf_stop(void)
{
    Continuous = 0;
    NewReading = 0;
    Enable = 0;
    WaitingForData = 0;
}


void rf_suspend(void)
{
    NewReading = 0;
    Enable = 0;
    WaitingForData = 0;
}


void rf_resume(void)
{
    if (Continuous)
        rf_run();
}


bool rf_waiting(void)
{
    return WaitingForData;
}


bool rf_continuous(void)
{
    return Continuous;
}


int rf_get_avg(void)
{
    return Averages;
}


void rf_set_avg(int n)
{
    if (n < 1)
        return;
    NewReading = 0;
    Averages = n;
    AverageAccu = 0;
    AverageAccuCount = 0; // re-start
    WaitingForData = 1;
}


bool rf_get_mdb(long *reading)
{
    if (!NewReading)
        return 0;
    NewReading = 0;
    
    if (!Enable)
        return 0;

    *reading = Reading;
    return 1;
}
