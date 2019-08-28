#include <nds.h>
#include <string.h>
#include "../../../common/sd_vram.h"
#include "../wifi/wifi.h"
#include "../wifi/wifi_tx.h"
#include "../wifi/wifi_rx.h"
#include "sio.h"
#include "sio_multi.h"

//#define SIO_MULTI_PRESEND

//static u16 sMultiData[4];
static bool sTransfering;
static u64 sStartTime;

#ifdef SIO_MULTI_PRESEND
static u16 sSlaveData;
static bool sReceivedSlaveData;
#endif

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
	//vram_cd->sioWork.sioMulti[0] = sMultiData[0];
	//sMultiData[0] = 0xFFFF;
	//vram_cd->sioWork.sioMulti[1] = sMultiData[1];
	//sMultiData[1] = 0xFFFF;
	//vram_cd->sioWork.sioMulti[2] = sMultiData[2];
	//sMultiData[2] = 0xFFFF;
	//vram_cd->sioWork.sioMulti[3] = sMultiData[3];
	//sMultiData[3] = 0xFFFF;
	vram_cd->sioWork.sioCntRead = (vram_cd->sioWork.sioCntRead & ~(SIO_MULTI_CNT_START_BUSY | SIO_MULTI_CNT_ERROR)) | (error ? SIO_MULTI_CNT_ERROR : 0);
	sTransfering = false;
	//raise irq
	if (vram_cd->sioWork.sioCnt & SIO_MULTI_CNT_IRQ)
	{
		//todo: maybe don't raise irq if disabled in IE register
		vram_cd->sioWork.sioIrqFlag = 1;
		*((vu32*)0x04000180) |= (1 << 13);
	}
}

static void txDoneCallback(void* arg)
{
	if (gSioWork.multiModes[gSioWork.id] != SIO_MODE_MULTI)
		return;

	/*if(gSioWork.id == SIO_ID_SLAVE_0 && sTransfering)
	{
		if (!(REG_WIFI_TXREQ_BUSY & WIFI_TXREQ_LOC3))
		{
			finishTransfer();
		}
	}*/

	//if (gSioWork.id == SIO_ID_MASTER && sTransfering && !(REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2) && *((u16*)&WIFI_RAM->txBuf[0]) == 0)
	//	finishTransfer(true);

#ifdef SIO_MULTI_PRESEND
	if (gSioWork.id == SIO_ID_MASTER && sTransfering)
	{
		if (!(REG_WIFI_TXREQ_BUSY & WIFI_TXREQ_LOC3))
		{
			vram_cd->sioWork.sioMulti[1] = sSlaveData;
			finishTransfer(false);
		}
	}
#endif
}


void sio_multiOnCmdRecv(u8 cmd, u8 arg0, u16 arg1, u16 arg2)
{
	if (gSioWork.multiModes[gSioWork.id] != SIO_MODE_MULTI)
		return;
	if(cmd == SIO_CMD_MODE_CHANGE)
	{
		if (sTransfering)
			finishTransfer(true);
		if ((arg0 & 7) == SIO_MODE_MULTI)
			vram_cd->sioWork.sioCntRead |= (1 << 3);
		else
			vram_cd->sioWork.sioCntRead &= ~(1 << 3);
		return;
	}
	if(cmd != SIO_CMD_DATA)
		return;
	u16 data = arg1;
	if (gSioWork.id == SIO_ID_MASTER)
	{
	#ifdef SIO_MULTI_PRESEND
		sSlaveData = data;
		sReceivedSlaveData = true;
	#else
		vram_cd->sioWork.sioMulti[SIO_ID_SLAVE_0] = data;
		finishTransfer(false);
	#endif				
	}
	else
	{
		vram_cd->sioWork.sioMulti[SIO_ID_MASTER] = data;
		vram_cd->sioWork.sioMulti[SIO_ID_SLAVE_0] = vram_cd->sioWork.sioMultiSend;

	#ifndef SIO_MULTI_PRESEND
		sTransfering = true;
		sStartTime = getUsCounter();
		vram_cd->sioWork.sioCntRead |= SIO_MULTI_CNT_START_BUSY;
	
		//send data to master
		sio_sendSimplePacket(&gSioWork.multiMacs[0], SIO_CMD_DATA, 0, vram_cd->sioWork.sioMultiSend, 0);
		// static struct { wifi_pkt_tx_t packet; u32 checksum; } packet =
		// {
		// 	{
		// 		{
		// 			0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 4
		// 		},
		// 		{
		// 			{0,2,0,0,0,0,0,0,0,0,0}, 0,
		// 			gSioWork.multiMacs[0],
		// 			//{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, //broadcast
		// 			WIFI_RAM->firmData.wifiData.macAddress, //source ds
		// 			{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},//WIFI_RAM->firmData.wifiData.macAddress,
		// 			//gSioWork.multiMacs[0], //mac address of master ds for ap filtering
		// 			{0,0}
		// 		}
		// 	},
		// 	0
		// };
		// packet.packet.ieeeHeader.addr3.address[0] = vram_cd->sioWork.sioMultiSend & 0xFF;
		// packet.packet.ieeeHeader.addr3.address[1] = vram_cd->sioWork.sioMultiSend >> 8;
		// while (REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2);
		// dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
		// wifi_setRetryLimit(7);
		// REG_WIFI_TXQ2_ADR = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
		// REG_WIFI_QUEUE_OPEN = WIFI_TXQ_TXQ_ALL;
	#endif
		finishTransfer(false);
		//sio_multiSendWrite(sMultiData[SIO_ID_MASTER]);
		//if(sMultiData[SIO_ID_MASTER] == 0x6200)
		//{
			//while (1);
		//}
		//while (1);
		//finishTransfer();
	}
}

void sio_multiStart()
{
    sTransfering = false;
    //sMultiData[0] = 0xFFFF;
    //sMultiData[1] = 0xFFFF;
    //sMultiData[2] = 0xFFFF;
    //sMultiData[3] = 0xFFFF;
	vram_cd->sioWork.sioMulti[0] = 0xFFFF;
	vram_cd->sioWork.sioMulti[1] = 0xFFFF;
	vram_cd->sioWork.sioMulti[2] = 0xFFFF;
	vram_cd->sioWork.sioMulti[3] = 0xFFFF;
	vram_cd->sioWork.sioIrqFlag = 0;
    vram_cd->sioWork.sioCntRead = (gSioWork.id << 4) | (gSioWork.id == SIO_ID_MASTER ? 0 : (1 << 2)) | (gSioWork.multiModes[gSioWork.id == SIO_ID_MASTER ? 1 : 0] == SIO_MODE_MULTI ? (1 << 3) : 0);
    wifi_setTxDoneCallback(txDoneCallback, NULL);
}

void sio_multiSendWrite(u16 data)
{
    //sMultiData[gSioWork.id] = data;
#ifdef SIO_MULTI_PRESEND
    if(gSioWork.id == SIO_ID_MASTER)
        return;

	//slaves immediately distribute data to the other dses
	static struct { wifi_pkt_tx_t packet; u8 payload[6]; } packet =
	{
		{
			{
				0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 6
			},
			{
				{0,2,0,0,0,0,0,0,0,0,0}, 0,
				gSioWork.multiMacs[0],
				//{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, //broadcast
				WIFI_RAM->firmData.wifiData.macAddress, //source ds
				{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},//WIFI_RAM->firmData.wifiData.macAddress,
				//gSioWork.multiMacs[0], //mac address of master ds for ap filtering
				{0,0}
			}
		},
		{
			//payload
			0, 0,
			//checksum
			0x00, 0x00, 0x00, 0x00
		}
	};
	packet.payload[0] = data & 0xFF;
	packet.payload[1] = data >> 8;
	while (REG_WIFI_TXREQ_BUSY & WIFI_TXREQ_LOC3);
	dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
	wifi_setRetryLimit(7);
	REG_WIFI_TXREQ_LOC3 = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
	REG_WIFI_TXREQ_EN_SET = WIFI_TXREQ_LOC_ALL;
#endif
}

void sio_multiCntWrite(u16 val)
{
	if (gSioWork.id == SIO_ID_MASTER && sTransfering && (getUsCounter() - sStartTime > 1000))
		finishTransfer(false);
    if(val & SIO_MULTI_CNT_START_BUSY && gSioWork.id == SIO_ID_MASTER && !sTransfering)
    {
        //start transfer
        sTransfering = true;
		sStartTime = getUsCounter();
        vram_cd->sioWork.sioCntRead = (vram_cd->sioWork.sioCntRead & ~SIO_MULTI_CNT_ERROR) | SIO_MULTI_CNT_START_BUSY;
		vram_cd->sioWork.sioMulti[0] = vram_cd->sioWork.sioMultiSend;// 0xFFFF;
		vram_cd->sioWork.sioMulti[1] = 0xFFFF;
        vram_cd->sioWork.sioMulti[2] = 0xFFFF;
		vram_cd->sioWork.sioMulti[3] = 0xFFFF;
		if(gSioWork.multiModes[SIO_ID_SLAVE_0] != SIO_MODE_MULTI)
			finishTransfer(false);
		else
			sio_sendSimplePacket(&gSioWork.multiMacs[1], SIO_CMD_DATA, 0, vram_cd->sioWork.sioMultiSend, 0);
		// static struct { wifi_pkt_tx_t packet; u32 checksum; } packet =
        // {
        //     {
        //         {
        //             0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 4
        //         },
        //         {
        //             {0,2,0,0,0,0,0,0,0,0,0}, 0, 
        //             //{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, //broadcast
		// 			gSioWork.multiMacs[1],
        //             WIFI_RAM->firmData.wifiData.macAddress, //source ds
		// 			{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},//WIFI_RAM->firmData.wifiData.macAddress,
        //             //WIFI_RAM->firmData.wifiData.macAddress, //mac address of master ds for ap filtering
        //             {0,0}
        //         }
        //     },
        //     0
        // };
		// packet.packet.ieeeHeader.addr3.address[0] = vram_cd->sioWork.sioMultiSend & 0xFF;
		// packet.packet.ieeeHeader.addr3.address[1] = vram_cd->sioWork.sioMultiSend >> 8;
		// while (REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2);
        // dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
		// wifi_setRetryLimit(7);
        // REG_WIFI_TXQ2_ADR = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
        // REG_WIFI_QUEUE_OPEN = WIFI_TXQ_TXQ_ALL;
    }
}
