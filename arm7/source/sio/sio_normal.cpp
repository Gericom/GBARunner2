#include <nds.h>
#include <string.h>
#include "../../../common/sd_vram.h"
#include "../wifi/wifi.h"
#include "../wifi/wifi_tx.h"
#include "../wifi/wifi_rx.h"
#include "sio.h"
#include "sio_normal.h"

//#define SIO_MULTI_PRESEND

//static u16 sMultiData[4];
static bool sTransfering;
static u64 sStartTime;

static u64 getUsCounter()
{
	u64 time = (u64)REG_WIFI_TSF_3 << 48;
	time |= (u64)REG_WIFI_TSF_2 << 32;
	time |= (u64)REG_WIFI_TSF_1 << 16;
	time |= (u64)REG_WIFI_TSF_0;
	return time;
}

static void finishTransfer(bool error)
{
	vram_cd->sioWork.sioCntRead &= ~SIO_NORMAL_CNT_START_BUSY;
	sTransfering = false;
	//raise irq
	if (vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_IRQ)
	{
		//todo: maybe don't raise irq if disabled in IE register
		vram_cd->sioWork.sioIrqFlag = 1;
		*((vu32*)0x04000180) |= (1 << 13);
	}
}

static void txDoneCallback(void* arg)
{
	if (gSioWork.multiModes[gSioWork.id] != SIO_MODE_NORMAL)
		return;
}


void sio_normalOnCmdRecv(u8 cmd, u8 arg0, u16 arg1, u16 arg2)
{
	if (gSioWork.multiModes[gSioWork.id] != SIO_MODE_NORMAL)
		return;
	if(cmd == SIO_CMD_MODE_CHANGE)
	{
		if (sTransfering)
			finishTransfer(true);
		// if ((arg0 & 7) == SIO_MODE_NORMAL)
		// 	vram_cd->sioWork.sioCntRead |= (1 << 3);
		// else
		// 	vram_cd->sioWork.sioCntRead &= ~(1 << 3);
		return;
	}
	if(cmd != SIO_CMD_DATA)
		return;
	u32 data = arg1 | (arg2 << 16);
	if (gSioWork.id == SIO_ID_MASTER)
	{
	#ifdef SIO_MULTI_PRESEND
		sSlaveData = data;
		sReceivedSlaveData = true;
	#else
        if(vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_USE_32BIT)
            vram_cd->sioWork.sioData32 = data;
        else
            vram_cd->sioWork.sioData8 = data & 0xFF;
		finishTransfer(false);
	#endif				
	}
	else
	{
		sTransfering = true;
		sStartTime = getUsCounter();
		vram_cd->sioWork.sioCntRead |= SIO_NORMAL_CNT_START_BUSY;
	
		//send data to master
		sio_sendSimplePacket(&gSioWork.multiMacs[0], SIO_CMD_DATA, 0, 
                (vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_USE_32BIT) ? (vram_cd->sioWork.sioData32 & 0xFFFF) : (vram_cd->sioWork.sioData8 & 0xFF),
                (vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_USE_32BIT) ? ((vram_cd->sioWork.sioData32 >> 16) & 0xFFFF) : 0);
        if(vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_USE_32BIT)
            vram_cd->sioWork.sioData32 = data;
        else
            vram_cd->sioWork.sioData8 = data & 0xFF;
		finishTransfer(false);
	}
}

void sio_normalStart()
{
    sTransfering = false;
	vram_cd->sioWork.sioIrqFlag = 0;
    vram_cd->sioWork.sioCntRead = 0;
    wifi_setTxDoneCallback(txDoneCallback, NULL);
}

void sio_normalCntWrite(u16 val)
{
	if (gSioWork.id == SIO_ID_MASTER && sTransfering && (getUsCounter() - sStartTime > 1000))
		finishTransfer(false);
    if(val & SIO_NORMAL_CNT_START_BUSY && gSioWork.id == SIO_ID_MASTER && !sTransfering)
    {
        //start transfer
        sTransfering = true;
		sStartTime = getUsCounter();
        vram_cd->sioWork.sioCntRead |= SIO_NORMAL_CNT_START_BUSY;
		if(gSioWork.multiModes[SIO_ID_SLAVE_0] != SIO_MODE_NORMAL)
			finishTransfer(false);
		else
			sio_sendSimplePacket(&gSioWork.multiMacs[1], SIO_CMD_DATA, 0, 
                (vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_USE_32BIT) ? (vram_cd->sioWork.sioData32 & 0xFFFF) : (vram_cd->sioWork.sioData8 & 0xFF),
                (vram_cd->sioWork.sioCnt & SIO_NORMAL_CNT_USE_32BIT) ? ((vram_cd->sioWork.sioData32 >> 16) & 0xFFFF) : 0);
    }
}
