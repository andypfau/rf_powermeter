#include "usb.h"
#include "helpers.h"
#include "mcc_generated_files/usb/usb.h"


#define BUFFER_SIZE 64
#define DATA_SIZE 64

static uint8_t RxBuffer[BUFFER_SIZE];

static uint8_t RxData[DATA_SIZE];
static uint8_t TxData[DATA_SIZE];
static int RxDataStart, RxDataSize;
static int TxDataSize;

static bool CommunicationStarted;


void usb_init(void)
{
    RxDataStart = 0;
    RxDataSize = 0;
    TxDataSize = 0;
    CommunicationStarted = 0;
}


bool usb_ready_to_send(void)
{
    return (TxDataSize == 0);
}


void usb_set_data(char *buffer, int size)
{
    if (TxDataSize != 0)
        return;
    for (int i = 0; i < size; i++)
    {
        if (i >= DATA_SIZE)
            break;
        TxData[i] = (uint8_t)buffer[i];
        TxDataSize = i + 1;
    }
}


int usb_get_data(char *buffer, int max_size)
{
    int result = 0;
    for (int i = 0; i < max_size; i++)
    {
        if (RxDataSize == 0)
            break;
        buffer[i] = (char)RxData[RxDataStart];
        result = i + 1;
        RxDataStart = (RxDataStart + 1) % DATA_SIZE;
        RxDataSize--;
    }
    return result;
}


void usb_loop(void)
{
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        return;
    }

    if( USBIsDeviceSuspended()== true )
    {
        return;
    }

    if( USBUSARTIsTxTrfReady() == true)
    {
        uint8_t nBytesReceived;
        uint8_t iRx;

        nBytesReceived = getsUSBUSART(RxBuffer, sizeof(RxBuffer));
        
        if (nBytesReceived != 0)
            CommunicationStarted = 1;
        
        for(iRx = 0; iRx < nBytesReceived; iRx++) {
            RxData[(RxDataStart+RxDataSize) % DATA_SIZE] = RxBuffer[iRx];
            RxDataSize++;
        }

        if (CommunicationStarted) {
            if(TxDataSize != 0) {
                putUSBUSART(TxData, TxDataSize);
                TxDataSize = 0;
            }
        }
    }

    CDCTxService();
}
