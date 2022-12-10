#include "infrastructure.h"
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/pin_manager.h"


static int AdcChannel;
static bool I2cAvailable;


void infra_init(void)
{
    infra_enable_rf(0);
    infra_enable_supply(0);
    infra_set_led(0);
    
    AdcChannel = 0;
    I2cAvailable = 1;
}


bool infra_get_button(void)
{
    return !IO_Button_GetValue(); // low=pressed
}


void infra_set_led(int leds)
{
    // low = on
    if (leds & IS_LED_R)
        IO_LedR_SetLow();
    else
        IO_LedR_SetHigh();
    if (leds & IS_LED_G)
        IO_LedG_SetLow();
    else
        IO_LedG_SetHigh();
    if (leds & IS_LED_B)
        IO_LedB_SetLow();
    else
        IO_LedB_SetHigh();
}


int infra_get_led(void) {
    // low = on
    int result = 0;
    if (!IO_LedR_GetValue())
        result |= IS_LED_R;
    if (!IO_LedG_GetValue())
        result |= IS_LED_G;
    if (!IO_LedB_GetValue())
        result |= IS_LED_B;
    return result;
}


void infra_enable_supply(bool enable)
{
    if (enable)
        IO_PwrEn_SetHigh();
    else
        IO_PwrEn_SetLow();
}


void infra_enable_rf(bool enable)
{
    if (enable)
        IO_EnRf_SetHigh();
    else
        IO_EnRf_SetLow();
}


// online docs for ADC: <https://ww1.microchip.com/downloads/en/DeviceDoc/39739b.pdf>


void infra_adc_sample(int channel)
{
    AdcChannel = channel;
    
    ADC1_Enable();
    ADC1_ChannelSelect(AdcChannel);
    
    // start sampling (not conversion)
    ADC1_SoftwareTriggerEnable();
}


void infra_adc_convert(void)
{
    // stop sampling, start conversion
    ADC1_SoftwareTriggerDisable();
}


bool infra_adc_done(void)
{
    return ADC1_IsConversionComplete(AdcChannel);
}


int infra_adc_get_raw_result(void)
{
    int result = (int)(ADC1_ConversionResultGet(AdcChannel));
    ADC1_Disable();
    return result;
}


int infra_adc_get_result(void)
{
    const float ExternalScalingTowardsAdc = 1.0 / 1.909; // all channels have the same scaling
    const float OutputScaling = 1000.0;
    const float Reference = 3.0;
    const uint32_t AdcMax = 0xFFFL;

    const int Shift = 9;
    const uint32_t Factor = (uint32_t)((1L << Shift) * OutputScaling * Reference / (ExternalScalingTowardsAdc * AdcMax));

    uint32_t raw = (uint32_t)(infra_adc_get_raw_result());
    uint16_t result = (uint16_t)((raw * Factor) >> Shift);
    
    return result;
}


bool infra_acquire_i2c(void)
{
    if (I2cAvailable)
    {
        I2cAvailable = 0;
        return 1;
    }
    else
        return 0;
}


void infra_release_i2c(void)
{
    I2cAvailable = 1;
}
