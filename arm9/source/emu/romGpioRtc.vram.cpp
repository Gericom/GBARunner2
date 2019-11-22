#include "sd_access.h"
#include "../../common/fifo.h"
#include "romGpio.h"
#include "romGpioRtc.h"

//based on https://github.com/mgba-emu/mgba/blob/master/src/gba/hardware.c

#define RTC_COMMAND_MAGIC           0x06
#define RTC_COMMAND_MAGIC_MASK      0x0F
#define RTC_COMMAND_CMD_MASK        0x70
#define RTC_COMMAND_CMD_SHIFT       4
#define RTC_COMMAND_READ            0x80

#define RTC_CMD_RESET       0
#define RTC_CMD_DATETIME    2
#define RTC_CMD_FORCE_IRQ   3
#define RTC_CMD_CONTROL     4
#define RTC_CMD_TIME        6

static u16 sRtcTransferState;
static u16 sRtcBits;
static u16 sRtcBitCount;
static u16 sRtcCommand;
static u16 sRtcCommandActive;
static u16 sRtcBytesLeft;

static u16 sRtcControl;

static u8 sRtcDateTime[8];

void rio_rtcInit()
{
    sRtcTransferState = 0;
    sRtcBits = 0;
    sRtcBitCount = 0;
    sRtcCommand = 0;
    sRtcCommandActive = 0;
    sRtcBytesLeft = 0;

    sRtcControl = 0x40;
}

static void updateDateTime()
{
    REG_SEND_FIFO = 0xAA550100;
    while (*((vu32*)0x04000184) & (1 << 8));
	*(u32*)&sRtcDateTime[0] = REG_RECV_FIFO;
    while (*((vu32*)0x04000184) & (1 << 8));
	*(u32*)&sRtcDateTime[4] = REG_RECV_FIFO;
    MI_WriteByte(&sRtcDateTime[4], sRtcDateTime[4] & 0x3F);
}

static void processByte()
{
    sRtcBytesLeft--;
    if(!sRtcCommandActive)
    {
        if((sRtcBits & RTC_COMMAND_MAGIC_MASK) == RTC_COMMAND_MAGIC)
        {
            sRtcCommand = sRtcBits;
            switch ((sRtcCommand & RTC_COMMAND_CMD_MASK) >> RTC_COMMAND_CMD_SHIFT)
            {
                case RTC_CMD_RESET:
                    sRtcControl = 0;
                    sRtcBytesLeft = 0;
                    break;
                case RTC_CMD_DATETIME:
                    updateDateTime();
                    sRtcBytesLeft = 7;
                    break;
                case RTC_CMD_FORCE_IRQ:
                    sRtcBytesLeft = 0;
                    break;
                case RTC_CMD_CONTROL:
                    sRtcBytesLeft = 1;
                    break;
                case RTC_CMD_TIME:
                    updateDateTime();
                    sRtcBytesLeft = 3;
                    break;
                default:
                    sRtcBytesLeft = 0;
                    break;
            }
            sRtcCommandActive = sRtcBytesLeft > 0;
        }
    }
    else
    {
        switch ((sRtcCommand & RTC_COMMAND_CMD_MASK) >> RTC_COMMAND_CMD_SHIFT)
        {
            case RTC_CMD_FORCE_IRQ:
                //todo
                break;
            case RTC_CMD_CONTROL:
                sRtcControl = sRtcBits;
                break;
        }
    }
    
    sRtcBits = 0;
    sRtcBitCount = 0;
    if(!sRtcBytesLeft)
    {
        sRtcCommandActive = 0;
        sRtcCommand = 0;
    }
}

static u32 getOutputBit()
{
    if(!sRtcCommandActive)
        return 0;
    u8 outByte = 0;
    switch ((sRtcCommand & RTC_COMMAND_CMD_MASK) >> RTC_COMMAND_CMD_SHIFT)
    {
        case RTC_CMD_DATETIME:
        case RTC_CMD_TIME:
            outByte = sRtcDateTime[7 - sRtcBytesLeft];
            break;
        case RTC_CMD_CONTROL:
            outByte = sRtcControl;
            break;
    }
    return (outByte >> sRtcBitCount) & 1;
}

void rio_rtcUpdate()
{
    switch(sRtcTransferState)
    {
        case 0:
            if((gRioGpioData & 5) == 1)
                sRtcTransferState = 1;
            break;
        case 1:            
            if((gRioGpioData & 5) == 5)
                sRtcTransferState = 2;
            else if((gRioGpioData & 5) != 1)
                sRtcTransferState = 0;
            break;
        case 2:
            if(!(gRioGpioData & 1))
                sRtcBits = (sRtcBits & ~(1 << sRtcBitCount)) | (((gRioGpioData >> 1) & 1) << sRtcBitCount);
            else if(gRioGpioData & 4)
            {
                if(sRtcCommand & RTC_COMMAND_READ)
                {
                    gRioGpioData = (gRioGpioData & gRioGpioDirection) | ((5 | (getOutputBit() << 1)) & ~gRioGpioDirection);
                    rio_invalidate();
                    if(++sRtcBitCount == 8)
                    {
                        sRtcBitCount = 0;
                        if(--sRtcBytesLeft <= 0)
                        {
                            sRtcCommandActive = 0;
                            sRtcCommand = 0;
                        }
                    }
                }
                else if(++sRtcBitCount == 8)
                    processByte();
            }
            else
            {
                sRtcBits = 0;
                sRtcBitCount = 0;
                sRtcCommandActive = 0;
                sRtcCommand = 0;
                sRtcBytesLeft = 0;
                sRtcTransferState = gRioGpioData & 1;
                gRioGpioData = (gRioGpioData & gRioGpioDirection) | (1 & ~gRioGpioDirection);
                rio_invalidate();
            }            
            break;
    }
}