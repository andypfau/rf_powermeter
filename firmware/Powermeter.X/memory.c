#include "memory.h"
#include "mcc_generated_files/i2c1.h"


#define MEMORY_ADDR 0b1010000

// maximum supported by memory is 128
#define MAX_WRITE_BYTES 8

static uint8_t TrbBuffer[2+MAX_WRITE_BYTES];
static I2C1_TRANSACTION_REQUEST_BLOCK Trb[2];
static I2C1_MESSAGE_STATUS Status;
static bool Active;
static bool AckPollingPending, AckPollingOngoing;


void mem_init(void)
{
    Active = 0;
    AckPollingPending = 0;
    AckPollingOngoing = 0;
}


bool mem_write(uint16_t address, int size, uint8_t *write_data_buffer)
{
    if (size > MAX_WRITE_BYTES)
        return 0; // too large for buffer
    
    long startPage = ((long)address) >> 7;
    long endPage = (((long)address) + ((long)size) - 1L) >> 7;
    if (startPage != endPage)
        return 0; // crossing pages during page write is not allowed
    
    TrbBuffer[0] = (address >> 8) & 0xFF;
    TrbBuffer[1] = (address     ) & 0xFF;
    for (int i = 0; i < size; i++)
        TrbBuffer[2+i] = write_data_buffer[i];
    
    Status = I2C1_MESSAGE_PENDING;
    I2C1_MasterWriteTRBBuild(&Trb[0], TrbBuffer, 2+size, MEMORY_ADDR);
    I2C1_MasterTRBInsert(1, Trb, &Status);
    
    AckPollingPending = 1;
    
    return 1;
}


bool mem_read(uint16_t address, int size, uint8_t *read_data_buffer)
{
    TrbBuffer[0] = (address >> 8) & 0xFF;
    TrbBuffer[1] = (address     ) & 0xFF;
    
    Status = I2C1_MESSAGE_PENDING;
    I2C1_MasterWriteTRBBuild(&Trb[0], TrbBuffer, 2, MEMORY_ADDR);
    I2C1_MasterReadTRBBuild(&Trb[1], read_data_buffer, size, MEMORY_ADDR);                
    I2C1_MasterTRBInsert(2, Trb, &Status);
    
    return 1;
}


int mem_wait(void)
{
    int status;
    
    while (1) {
        status = mem_check();
        if (status <= 0)
            return status;
    }
}


void do_ack_polling(void)
{
    Status = I2C1_MESSAGE_PENDING;
    I2C1_MasterWriteTRBBuild(&Trb[0], TrbBuffer, 2, MEMORY_ADDR); // contents of the buffer don't matter
    I2C1_MasterTRBInsert(1, &Trb[0], &Status);
}


int mem_check(void)
{
    switch(Status) {
        case I2C1_MESSAGE_COMPLETE:
            if (AckPollingPending) {
                do_ack_polling();
                AckPollingPending = 0;
                AckPollingOngoing = 1;
                return MEM_I2C_STATUS_ACKP;
            } else {
                AckPollingOngoing = 0;
                return MEM_I2C_STATUS_OK;
            }
        case I2C1_MESSAGE_PENDING:
            return MEM_I2C_STATUS_PENDING;
        case I2C1_MESSAGE_FAIL:
            AckPollingOngoing = 0;
            AckPollingOngoing = 0;
            return MEM_I2C_ERROR_FAIL;
        case I2C1_STUCK_START:
            AckPollingOngoing = 0;
            AckPollingOngoing = 0;
            return MEM_I2C_ERROR_STUCK;
        case I2C1_MESSAGE_ADDRESS_NO_ACK:
            if (AckPollingOngoing) {
                do_ack_polling();
                return MEM_I2C_STATUS_ACKP;
            } else
                return MEM_I2C_STATUS_RETRY_ANA;
        case I2C1_DATA_NO_ACK:
            if (AckPollingOngoing) {
                do_ack_polling();
                return MEM_I2C_STATUS_ACKP;
            } else
                return MEM_I2C_STATUS_RETRY_DNA;
        case I2C1_LOST_STATE:
            AckPollingOngoing = 0;
            AckPollingOngoing = 0;
            return MEM_I2C_ERROR_LOST;
        default:
            AckPollingOngoing = 0;
            AckPollingOngoing = 0;
            return MEM_I2C_ERROR_UNKNOWN;
    }
}
