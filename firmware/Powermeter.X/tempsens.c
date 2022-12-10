#include "tempsens.h"
#include "mcc_generated_files/i2c1.h"


static uint8_t TxData[4];
static uint8_t RxData[4];
static I2C1_TRANSACTION_REQUEST_BLOCK Trb[2];
static I2C1_MESSAGE_STATUS Status;


#define TEMP_SENS_ADDR 0b1001000


// online docs: <https://microchipdeveloper.com/xpress:how-to:on-board-temperature-sensor>


void temp_init(void)
{
    // nothing to do
}


void temp_convert(void)
{
    TxData[0] = 0x00; // read temperature register

    Status = I2C1_MESSAGE_PENDING;
    I2C1_MasterWriteTRBBuild(&Trb[0], TxData, 1, TEMP_SENS_ADDR);
    I2C1_MasterReadTRBBuild(&Trb[1], RxData, 2, TEMP_SENS_ADDR);                
    I2C1_MasterTRBInsert(2, &Trb[0], &Status);
}


bool temp_done(void)
{
    return (Status != I2C1_MESSAGE_PENDING);
}


bool temp_ok(void)
{
    return (Status == I2C1_MESSAGE_COMPLETE);
}


int temp_get_raw_result(void)
{
    uint16_t result =
        ((uint16_t)(RxData[0]) << 8) |
        ((uint16_t)(RxData[1]));
    return result;
}


long temp_get_result_mdeg(void)
{
    const float ExternalScalingTowardsAdc = 1.0;
    const float OutputScaling = 1000.0;
    const float Reference = 128;
    const uint32_t AdcMax = 0x7FF0L;

    const int Shift = 15;
    const uint32_t Factor = (uint32_t)((1L << Shift) * OutputScaling * Reference / (ExternalScalingTowardsAdc * AdcMax));

    uint32_t raw = (uint32_t)(temp_get_raw_result());
    uint32_t result = (uint32_t)((raw * Factor) >> Shift);
    
    return result;
}
