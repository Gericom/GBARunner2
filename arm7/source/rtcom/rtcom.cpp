#include <nds.h>
#include "../../../common/sd_vram.h"
#include "rtcom.h"

#ifdef USE_3DS_32MB

// Delay (in swiDelay units) for each bit transfer
#define RTC_DELAY 48

// Pin defines on RTC_CR
#define CS_0    (1<<6)
#define CS_1    ((1<<6) | (1<<2))
#define SCK_0   (1<<5)
#define SCK_1   ((1<<5) | (1<<1))
#define SIO_0   (1<<4)
#define SIO_1   ((1<<4) | (1<<0))
#define SIO_out (1<<4)
#define SIO_in  (1)

#define RTC_READ_112 	0x6D
#define RTC_WRITE_112	0x6C

#define RTC_READ_113    0x6F
#define RTC_WRITE_113	0x6E

static void waitByLoop(volatile int count)
{
	while(--count);
}

static void rtcTransferReversed(u8* cmd, u32 cmdLen, u8* result, u32 resultLen) 
{
	// Raise CS
	RTC_CR8 = CS_0 | SCK_1 | SIO_1;
	waitByLoop(2);
	RTC_CR8 = CS_1 | SCK_1 | SIO_1;
	waitByLoop(2);

	// Write command byte (high bit first)
    u8 data = *cmd++;

    for (u32 bit = 0; bit < 8; bit++) 
    {
        RTC_CR8 = CS_1 | SCK_0 | SIO_out | (data >> 7);
		waitByLoop(9);

        RTC_CR8 = CS_1 | SCK_1 | SIO_out | (data >> 7);
		waitByLoop(9);

        data <<= 1;
    }
	// Write parameter bytes (high bit first)
	for (; cmdLen > 1; cmdLen--) 
    {
		data = *cmd++;
		for (u32 bit = 0; bit < 8; bit++) 
        {
			RTC_CR8 = CS_1 | SCK_0 | SIO_out | (data >> 7);
			waitByLoop(9);

			RTC_CR8 = CS_1 | SCK_1 | SIO_out | (data >> 7);
			waitByLoop(9);

			data <<= 1;
		}
	}

	// Read result bytes (high bit first)
	for (; resultLen > 0; resultLen--)
    {
		data = 0;
		for (u32 bit = 0; bit < 8; bit++) 
        {
			RTC_CR8 = CS_1 | SCK_0;
			waitByLoop(9);

			RTC_CR8 = CS_1 | SCK_1;
			waitByLoop(9);

            data <<= 1;
			if (RTC_CR8 & SIO_in)
                data |= 1;
		}
		*result++ = data;
	}

	// Finish up by dropping CS low
	waitByLoop(2);
	RTC_CR8 = CS_0 | SCK_1;
	waitByLoop(2);
}

static u8 readReg112()
{
	u8 readCmd = RTC_READ_112;
	u8 readVal = 0;
	rtcTransferReversed(&readCmd, 1, &readVal, 1);
	return readVal;
}

static void writeReg112(u8 val)
{
	u8 command[2] = { RTC_WRITE_112, val };
	rtcTransferReversed(command, 2, 0, 0);
}

static u8 readReg113()
{
	u8 readCmd = RTC_READ_113;
	u8 readVal = 0;
	rtcTransferReversed(&readCmd, 1, &readVal, 1);
	return readVal;
}

static void writeReg113(u8 val)
{
	u8 command[2] = { RTC_WRITE_113, val };
	rtcTransferReversed(command, 2, 0, 0);
}

static u16 oldRcnt;

void rtcom_beginComm()
{
	oldRcnt = REG_RCNT;
	REG_IF = IRQ_NETWORK;
	REG_RCNT = 0x8100; //enable irq
	REG_IF = IRQ_NETWORK;
}

void rtcom_endComm()
{
	REG_IF = IRQ_NETWORK;
	REG_RCNT = oldRcnt;
}

u8 rtcom_getData()
{
	return readReg112();
}

bool rtcom_waitStatus(u8 status)
{
	int timeout = 2062500;
    do
    {
		if(!(REG_IF & IRQ_NETWORK))
			continue;

		REG_IF = IRQ_NETWORK;
		return status == readReg113();
    }
    while(--timeout);

	REG_IF = IRQ_NETWORK;
    return false;
}

void rtcom_requestAsync(u8 request)
{
	writeReg113(request);
}

void rtcom_requestAsync(u8 request, u8 param)
{
	writeReg112(param);
	writeReg113(request);
}

bool rtcom_request(u8 request)
{
	rtcom_requestAsync(request);
	return rtcom_waitAck();
}

bool rtcom_request(u8 request, u8 param)
{
	rtcom_requestAsync(request, param);
	return rtcom_waitAck();
}

bool rtcom_requestKill()
{
	rtcom_requestAsync(0xFE);
	return rtcom_waitReady();
}

void rtcom_signalDone()
{
	writeReg113(RTCOM_STAT_DONE);
}

u32 rtcom_getProcAddr(u32 id)
{
	//todo
	return 0;
}

bool rtcom_uploadUCode(const void* uCode, u32 length)
{
	if(!rtcom_request(RTCOM_REQ_UPLOAD_UCODE, length & 0xFF))
		return false;

	if(!rtcom_requestNext((length >> 8) & 0xFF))
		return false;

	if(!rtcom_requestNext((length >> 16) & 0xFF))
		return false;

	if(!rtcom_requestNext((length >> 24) & 0xFF))
		return false;

	const u8* pCode = (const u8*)uCode;
	for(u32 i = 0; i < length; i++)
		if(!rtcom_requestNext(*pCode++))
			return false;

	//make it executable
	return rtcom_request(RTCOM_REQ_FINISH_UCODE);
}

bool rtcom_executeUCode(u8 param)
{
	return rtcom_request(RTCOM_REQ_EXECUTE_UCODE, param);
}

#endif