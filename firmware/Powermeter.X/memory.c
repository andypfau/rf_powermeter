#include "memory.h"
#include "mcc_generated_files/i2c1.h"


#define MEMORY_ADDR 0b1010000

// maximum supported by memory is 128
#define MAX_WRITE_BYTES 8

static uint8_t Buffer[2+MAX_WRITE_BYTES];
static I2C1_TRANSACTION_REQUEST_BLOCK Trb[2];
static I2C1_MESSAGE_STATUS Status;
static bool Active;


void mem_init(void)
{
    Active = 0;
}


void mem_write(int address, int size, uint8_t *buffer)
{
    if (size > MAX_WRITE_BYTES)
        return; // too large for buffer
    if ((address/128) != ((address+size-1)/128))
        return; // crossing pages during page write is not allowed
    
    Buffer[0] = (address >> 8) & 0xFF;
    Buffer[1] = (address     ) & 0xFF;
    for (int i = 0; i < size; i++)
        Buffer[2+i] = buffer[i];
    
    Status = I2C1_MESSAGE_PENDING;
    I2C1_MasterWriteTRBBuild(&Trb[0], Buffer, 2+size, MEMORY_ADDR);
    I2C1_MasterTRBInsert(1, &Trb[0], &Status);
}


void mem_read(int address, int size, uint8_t *buffer)
{
    Buffer[0] = (address >> 8) & 0xFF;
    Buffer[1] = (address     ) & 0xFF;
    
    Status = I2C1_MESSAGE_PENDING;
    I2C1_MasterWriteTRBBuild(&Trb[0], Buffer, 2, MEMORY_ADDR);
    I2C1_MasterReadTRBBuild(&Trb[1], buffer, size, MEMORY_ADDR);                
    I2C1_MasterTRBInsert(2, &Trb[0], &Status);
}


void mem_wait(void)
{
    while (!mem_done())
    {
        // wait...
    }
}


bool mem_done(void)
{
    if (Status == I2C1_MESSAGE_COMPLETE) {
        // acknowledged -> done
        return 1;
    }
    
    if (Status != I2C1_MESSAGE_PENDING) {
        // try again (ack-polling)
        Status = I2C1_MESSAGE_PENDING;
        I2C1_MasterWriteTRBBuild(&Trb[0], Buffer, 2, MEMORY_ADDR);
        I2C1_MasterTRBInsert(1, &Trb[0], &Status);
    }
    
    return 0;
}
