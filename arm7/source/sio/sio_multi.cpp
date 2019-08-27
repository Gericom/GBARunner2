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
	vram_cd->sioWork.sioCntRead = (vram_cd->sioWork.sioCntRead & ~SIO_MULTI_CNT_START_BUSY) | (error ? SIO_MULTI_CNT_ERROR : 0);
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
	if (gSioWork.mode != SIO_MODE_MULTI)
		return;

	/*if(gSioWork.id == SIO_ID_SLAVE_0 && sTransfering)
	{
		if (!(REG_WIFI_TXREQ_BUSY & WIFI_TXREQ_LOC3))
		{
			finishTransfer();
		}
	}*/

	if (gSioWork.id == SIO_ID_MASTER && sTransfering && !(REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2) && *((u16*)&WIFI_RAM->txBuf[0]) == 0)
		finishTransfer(true);

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
	if (gSioWork.mode != SIO_MODE_MULTI)
		return;
	int count = 0;
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
			u16 data = rxRamRead(base + 12 + 16);
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
				static struct { wifi_pkt_tx_t packet; u32 checksum; } packet =
				{
					{
						{
							0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 4
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
					0
				};
				packet.packet.ieeeHeader.addr3.address[0] = vram_cd->sioWork.sioMultiSend & 0xFF;
				packet.packet.ieeeHeader.addr3.address[1] = vram_cd->sioWork.sioMultiSend >> 8;
				while (REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2);
				dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
				wifi_setRetryLimit(7);
				REG_WIFI_TXQ2_ADR = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
				REG_WIFI_QUEUE_OPEN = WIFI_TXQ_TXQ_ALL;
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
			/*struct { wifi_pkt_tx_t packet; u8 payload[6]; } packet =
			{
				{
					{
						0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 6
					},
					{
						{0,2,0,0,0,0,0,0,0,0,0}, 0,
						{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, //broadcast
						WIFI_RAM->firmData.wifiData.macAddress, //source ds
						{0, 0, 0, 0, 0, 0},//WIFI_RAM->firmData.wifiData.macAddress,
						//gSioWork.multiMacs[0], //mac address of master ds for ap filtering
						{0,0}
					}
				},
				{
					//payload
					data & 0xFF, data >> 8,
					//checksum
					0x00, 0x00, 0x00, 0x00
				}
			};
			while (REG_WIFI_TXREQ_BUSY & WIFI_TXREQ_LOC3);
			dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
			REG_WIFI_TXREQ_LOC3 = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
			REG_WIFI_TXREQ_EN_SET = WIFI_TXREQ_LOC_ALL;*/
		}

		REG_WIFI_RXBUF_BNR = wrapRxPtr(base + len) >> 1;

		//if (++count == 6)
		//	break;
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
    vram_cd->sioWork.sioCntRead = (gSioWork.id << 4) | (1 << 3) | (gSioWork.id == SIO_ID_MASTER ? 0 : (1 << 2));
    wifi_setTxDoneCallback(txDoneCallback, NULL);
	wifi_setRxDoneCallback(rxDoneCallback, NULL);
	REG_WIFI_BUFFERING_SELECT = 0x0FFF;//0x381;
	REG_WIFI_DS_MASK = 8;
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
	if (gSioWork.id == SIO_ID_MASTER && sTransfering && (getUsCounter() - sStartTime > 16000))
		finishTransfer(true);
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
		static struct { wifi_pkt_tx_t packet; u32 checksum; } packet =
        {
            {
                {
                    0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 4
                },
                {
                    {0,2,0,0,0,0,0,0,0,0,0}, 0, 
                    //{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, //broadcast
					gSioWork.multiMacs[1],
                    WIFI_RAM->firmData.wifiData.macAddress, //source ds
					{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},//WIFI_RAM->firmData.wifiData.macAddress,
                    //WIFI_RAM->firmData.wifiData.macAddress, //mac address of master ds for ap filtering
                    {0,0}
                }
            },
            0
        };
		packet.packet.ieeeHeader.addr3.address[0] = vram_cd->sioWork.sioMultiSend & 0xFF;
		packet.packet.ieeeHeader.addr3.address[1] = vram_cd->sioWork.sioMultiSend >> 8;
		while (REG_WIFI_TXREQ_BUSY & WIFI_TXQ_TXQ2);
        dmaCopyWords(3, &packet, (void*)&WIFI_RAM->txBuf[0], sizeof(packet));
		wifi_setRetryLimit(7);
        REG_WIFI_TXQ2_ADR = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
        REG_WIFI_QUEUE_OPEN = WIFI_TXQ_TXQ_ALL;
    }
}
