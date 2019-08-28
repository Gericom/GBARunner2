#include <nds.h>
#include <string.h>
#include "../../../common/sd_vram.h"
#include "../wifi/wifi.h"
#include "../wifi/wifi_tx.h"
#include "../wifi/wifi_rx.h"
#include "sio_multi.h"
#include "sio.h"

sio_arm7_work_t gSioWork;

void sio_updateMode(u16 cnt, u16 rcnt)
{
    int mode;
    if(rcnt & 0x8000)
    {
        if(rcnt & 0x4000)
            mode = SIO_MODE_JOY;
        else
            mode = SIO_MODE_GPIO;
    }
    else
    {
        switch((cnt >> 12) & 3)
        {
            case 0:
                mode = SIO_MODE_NORMAL;
                break;
            case 1:
                mode = SIO_MODE_NORMAL;
                break;
            case 2:
                mode = SIO_MODE_MULTI;
                break;
            case 3:
                mode = SIO_MODE_UART;
                break;
        }
    }
    if(mode != gSioWork.multiModes[gSioWork.id])
    {
        gSioWork.multiModes[gSioWork.id] = mode;
        //mode switch
        switch(mode)
        {
            case SIO_MODE_GPIO:
            case SIO_MODE_NORMAL:
            case SIO_MODE_JOY:
            case SIO_MODE_UART:
                vram_cd->sioWork.sioCntRead = (gSioWork.id == SIO_ID_MASTER ? 0 : (1 << 2));
				//REG_WIFI_BUFFERING_SELECT = 0;
				wifi_setTxDoneCallback(NULL, NULL);
                break;
            case SIO_MODE_MULTI:
                sio_multiStart();
                break;
        }
        sio_sendSimplePacket(&gSioWork.multiMacs[gSioWork.id == SIO_ID_MASTER ? 1 : 0], SIO_CMD_MODE_CHANGE, (gSioWork.id << 6) | gSioWork.multiModes[gSioWork.id], 0, 0);
    }
}

static void writeCntReg(u16 val)
{
    sio_updateMode(val, vram_cd->sioWork.rcnt);
    switch(gSioWork.multiModes[gSioWork.id])
    {
        case SIO_MODE_GPIO:
        case SIO_MODE_NORMAL:
        case SIO_MODE_JOY:
            break;
        case SIO_MODE_MULTI:
            sio_multiCntWrite(val);
            break;
    }
}

static void writeRcntReg(u16 val)
{
    sio_updateMode(vram_cd->sioWork.sioCnt, val);
}

void sio_writeReg16(u16 addr, u16 val)
{
    if(addr == 0x128)
        writeCntReg(val);
    else if(addr == 0x134)
        writeRcntReg(val);
    else if(gSioWork.multiModes[gSioWork.id] == SIO_MODE_MULTI && addr == 0x12A)
        sio_multiSendWrite(val);
}

static u32 wrapRxPtr(u32 ptr)
{
	u32 start = REG_WIFI_RXBUF_STR & 0x1FFE;
	u32 end = REG_WIFI_RXBUF_END & 0x1FFE;
	if (ptr >= end)
		ptr -= end - start;
	return ptr;
}

static u16 rxRamRead(u32 offset)
{	
	return *(u16*)(((u32)WIFI_RAM) + wrapRxPtr(offset));
}

static void rxDoneCallback(void* arg)
{
    while(REG_WIFI_RXBUF_CUR != REG_WIFI_RXBUF_BNR)
	{
		int base = REG_WIFI_RXBUF_BNR << 1;
		int hdrLen = rxRamRead(base + 8);
		int len = ((hdrLen + 3) & ~3) + 12;

		if((gSioWork.id == SIO_ID_MASTER && rxRamRead(base + 12 + 4 + 6 + 0) == gSioWork.multiMacs[1].address16[0] && 
											rxRamRead(base + 12 + 4 + 6 + 2) == gSioWork.multiMacs[1].address16[1] &&
											rxRamRead(base + 12 + 4 + 6 + 4) == gSioWork.multiMacs[1].address16[2]) ||
			(gSioWork.id == SIO_ID_SLAVE_0 && rxRamRead(base + 12 + 4 + 6 + 0) == gSioWork.multiMacs[0].address16[0] &&
											  rxRamRead(base + 12 + 4 + 6 + 2) == gSioWork.multiMacs[0].address16[1] &&
											  rxRamRead(base + 12 + 4 + 6 + 4) == gSioWork.multiMacs[0].address16[2]))
		{
            u16 a = rxRamRead(base + 12 + 16);
            u8 cmd = a & 0xFF;
            u8 arg0 = (a >> 8) & 0xFF;
            u16 arg1 = rxRamRead(base + 12 + 18);
            u16 arg2 = rxRamRead(base + 12 + 20);
            if(cmd == SIO_CMD_MODE_CHANGE)
                gSioWork.multiModes[(arg0 >> 6) & 3] = arg0 & 7;
            switch(gSioWork.multiModes[gSioWork.id])
            {
                case SIO_MODE_GPIO:
                case SIO_MODE_NORMAL:
                case SIO_MODE_JOY:
                case SIO_MODE_UART:
                    break;
                case SIO_MODE_MULTI:
                    sio_multiOnCmdRecv(cmd, arg0, arg1, arg2);
                    break;
            }			
		}
		REG_WIFI_RXBUF_BNR = wrapRxPtr(base + len) >> 1;
	}
}

void sio_init()
{
    memset(&gSioWork, 0, sizeof(gSioWork));
	if (WIFI_RAM->firmData.wifiData.macAddress.address[0] == vram_cd->sioWork.slaveMac[0] &&
		WIFI_RAM->firmData.wifiData.macAddress.address[1] == vram_cd->sioWork.slaveMac[1] &&
		WIFI_RAM->firmData.wifiData.macAddress.address[2] == vram_cd->sioWork.slaveMac[2] &&
		WIFI_RAM->firmData.wifiData.macAddress.address[3] == vram_cd->sioWork.slaveMac[3] &&
		WIFI_RAM->firmData.wifiData.macAddress.address[4] == vram_cd->sioWork.slaveMac[4] &&
		WIFI_RAM->firmData.wifiData.macAddress.address[5] == vram_cd->sioWork.slaveMac[5])
	{
		gSioWork.id = SIO_ID_SLAVE_0;
		gSioWork.multiMacs[0].address[0] = vram_cd->sioWork.masterMac[0];
		gSioWork.multiMacs[0].address[1] = vram_cd->sioWork.masterMac[1];
		gSioWork.multiMacs[0].address[2] = vram_cd->sioWork.masterMac[2];
		gSioWork.multiMacs[0].address[3] = vram_cd->sioWork.masterMac[3];
		gSioWork.multiMacs[0].address[4] = vram_cd->sioWork.masterMac[4];
		gSioWork.multiMacs[0].address[5] = vram_cd->sioWork.masterMac[5];
		gSioWork.multiMacs[1] = WIFI_RAM->firmData.wifiData.macAddress;

		REG_WIFI_BSSID.address16[0] = 0;
		REG_WIFI_BSSID.address16[1] = 0;
		REG_WIFI_BSSID.address16[2] = 0;
	}
	else
	{
		gSioWork.id = SIO_ID_MASTER;
		gSioWork.multiMacs[0] = WIFI_RAM->firmData.wifiData.macAddress;
		gSioWork.multiMacs[1].address[0] = vram_cd->sioWork.slaveMac[0];
		gSioWork.multiMacs[1].address[1] = vram_cd->sioWork.slaveMac[1];
		gSioWork.multiMacs[1].address[2] = vram_cd->sioWork.slaveMac[2];
		gSioWork.multiMacs[1].address[3] = vram_cd->sioWork.slaveMac[3];
		gSioWork.multiMacs[1].address[4] = vram_cd->sioWork.slaveMac[4];
		gSioWork.multiMacs[1].address[5] = vram_cd->sioWork.slaveMac[5];

		REG_WIFI_BSSID.address16[0] = 0;
		REG_WIFI_BSSID.address16[1] = 0;
		REG_WIFI_BSSID.address16[2] = 0;
	}
	wifi_setRxDoneCallback(rxDoneCallback, NULL);
    REG_WIFI_BUFFERING_SELECT = 0x0FFF;//0x381;
	REG_WIFI_DS_MASK = 8;	
}

void sio_sendSimplePacket(wifi_macaddr_t* dst, u8 cmd, u8 arg0, u16 arg1, u16 arg2)
{
    struct { wifi_pkt_tx_t packet; u32 checksum; } packet =
    {
        {
            {
                0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 4
            },
            {
                {0,2,0,0,0,0,0,0,0,0,0}, 0, 
                //{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, //broadcast
                *dst,
                WIFI_RAM->firmData.wifiData.macAddress, //source ds
                {cmd, arg0, (u8)(arg1 & 0xFF), (u8)((arg1 >> 8) & 0xFF), (u8)(arg2 & 0xFF), (u8)((arg2 >> 8) & 0xFF)},//WIFI_RAM->firmData.wifiData.macAddress,
                //WIFI_RAM->firmData.wifiData.macAddress, //mac address of master ds for ap filtering
                {0,0}
            }
        },
        0
    };
    while (REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2);
    dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
    wifi_setRetryLimit(7);
    REG_WIFI_TXQ2_ADR = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
    REG_WIFI_QUEUE_OPEN = WIFI_TXQ_TXQ_ALL;
}