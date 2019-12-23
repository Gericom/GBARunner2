#include <nds.h>
#include <string.h>
#include "timer.h"
#include "sound.h"
#include "gbsound.h"
#include "save.h"
#include "dldi_handler.h"
#include "../../common/fifo.h"
#include "../../common/common_defs.s"
#include "wifi/wifi.h"
#include "wifi/wifi_tx.h"
#include "sio/sio.h"
#include "rtc.h"

static bool sPrevTouchDown = false;

extern "C" void __libnds_exit()
{

}

extern "C" void irq_vblank()
{
	bool touchDown = (REG_KEYXY & (1 << 6)) == 0;
	if(!sPrevTouchDown && touchDown)
	{
		//invoke irq on arm9
		vram_cd->openMenuIrqFlag = 1;
		REG_IPC_SYNC |= IPC_SYNC_IRQ_REQUEST;
	}
	sPrevTouchDown = touchDown;
}

extern "C" void my_irq_handler();

int main()
{
	//dmaFillWords(0, (void*)0x04000400, 0x100);

	//REG_SOUNDCNT |= SOUND_ENABLE;
	//writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	//powerOn(POWER_SOUND);

	REG_IME = 0;
	REG_IE = 0;
	REG_IF = ~0;

	*((vu32*)0x0380FFFC) = (vu32)&my_irq_handler;

#if defined(USE_DSI_16MB)
	//enable 16 MB mode
	*((vu32*)0x04004008) = (*((vu32*)0x04004008) & ~(3 << 14)) | (2 << 14); 
#elif defined(USE_3DS_32MB)
	//enable 32 MB mode
	*((vu32*)0x04004008) = (*((vu32*)0x04004008) & ~(3 << 14)) | (3 << 14); 
#endif

	REG_IME = 1;

	//try wifi
	wifi_init();
	wifi_start();
	sio_init();

	
	/*struct{wifi_pkt_tx_t packet; u8 payload[8];} sTestPacket =
	{
		{
			{
				0, 0, 0, 0, 0x14, sizeof(wifi_pkt_ieee_header_t) + 8
			},
			{
				{0,0,0,0,0,0,0,0,0,0,0}, 0, {0}, WIFI_RAM->firmData.wifiData.macAddress, {0}, {0,0}
			}
		},
		{
			//payload
			0x01, 0xAA, 0x55, 0xFF,
			//checksum
			0x00, 0x00, 0x00, 0x00
		}
	};
	while(true)
	{
		dmaCopyWords(3, &sTestPacket, (void*)&WIFI_RAM->txBuf[0], sizeof(sTestPacket));
		REG_WIFI_TXREQ_LOC3 = 0x8000 | (WIFI_RAM_TX_BUF_OFFSET >> 1);
		REG_WIFI_TXREQ_EN_SET = WIFI_TXREQ_LOC_ALL;
		while(*((vu16*)&WIFI_RAM->txBuf[0]) == 0 && *((vu16*)&WIFI_RAM->txBuf[2]) == 0);
		swiDelay(500 * 0x20BA); 
	}*/

	//*((vu16*)0x04000004) = 0x20 | (160 << 8);
	//*((vu16*)0x04000004) = 0x10;

	//irqInit();

	//REG_IE = 0;
	//REG_IF = ~0;

	//irqSet(IRQ_HBLANK, hblank_handler);
	//irqEnable(IRQ_HBLANK);
	//irqSet(IRQ_VBLANK, vblank_handler);
	//irqEnable(IRQ_VBLANK);

	//enable the arm7-arm9 fifo
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_SEND_CLEAR;
	//wait for the arm9 sync command
	do
	{
		while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
	}
	while (REG_RECV_FIFO != 0xAA5555AA);
#ifdef ARM7_DLDI
	while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
	uint8_t* dldi_src = (uint8_t*)REG_RECV_FIFO;
	memcpy((void*)0x0380A800, dldi_src, 16 * 1024);
	if(!dldi_handler_init())
	{
		REG_SEND_FIFO = 0x46494944;
		while(1);
	}
#endif

	//int oldirq = enterCriticalSection();
	//irqSet(IRQ_HBLANK, hblank_handler);

	gba_sound_init();
	gbs_init();
	gba_save_init();

	//set vblank irq
	REG_DISPSTAT |= DISP_VBLANK_IRQ;
	REG_IE |= IRQ_VBLANK;

	//irqSet(IRQ_VBLANK, vblank_irq_handler);
	//irqEnable(IRQ_VBLANK);

	//irqSet(IRQ_FIFO_NOT_EMPTY, fifo_handler);
	//irqEnable(IRQ_FIFO_NOT_EMPTY);
	//REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ;

	//leaveCriticalSection(oldirq);

	//send done command to arm9
	REG_SEND_FIFO = 0x55AAAA55;

	//REG_IE |= 1 << 2; //enable vcount interrupt
	//REG_IE |= 1 << 1; //enable hblank interrupt

	//REG_TM[2].CNT_H = 0;
	//irqDisable(IRQ_TIMER2);
	//irqSet(IRQ_TIMER2, timer_handler);
	//REG_TM[2].CNT_L = TIMER_FREQ(60)/64;//4 * 15734);
	////REG_TM[2].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_64;
	//irqEnable(IRQ_TIMER2);

	//REG_SOUND[0].SAD = 0x23F8000;
	//REG_SOUND[0].TMR = (u16)-1594; //-1253
	//REG_SOUND[0].PNT = 0;
	//REG_SOUND[0].LEN = 396 * 10;
	//REG_SOUND[0].CNT = SOUND_CHANNEL_0_SETTINGS;
	//fifo loop
	//vu32 val;
	//int frameOffset = 0;
	while (1)
	{
		while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
		{
			if (!(REG_KEYXY & 1))
				gba_sound_resync();
		}

		u32 cmd = REG_RECV_FIFO;
		//if((cmd >> 16) != 0xAA55)
		//	continue;
		uint32_t vals[4];
		u32      val;
		switch (cmd)
		{
			case 0xAA5500C4: //fifo_start_sound_command
				//REG_SOUND[0].CNT |= REG_SOUNDXCNT_E;
				gba_sound_notify_reset();
				break;
#ifdef ARM7_DLDI
			case 0xAA5500DF:
			{
				while(REG_FIFO_CNT & FIFO_CNT_EMPTY);
				uint32_t sector = REG_RECV_FIFO;
				while(REG_FIFO_CNT & FIFO_CNT_EMPTY);
				uint32_t count = REG_RECV_FIFO;
				while(REG_FIFO_CNT & FIFO_CNT_EMPTY);
				uint8_t* dst = (uint8_t*)REG_RECV_FIFO;
				dldi_handler_read_sectors(sector, count, dst);
				REG_SEND_FIFO = 0x55AAAA55;
				break;
			}
			case 0xAA5500F0:
			{
				while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
				uint32_t sector = REG_RECV_FIFO;
				while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
				uint32_t count = REG_RECV_FIFO;
				while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
				uint8_t* src = (uint8_t*)REG_RECV_FIFO;
				dldi_handler_write_sectors(sector, count, src);
				REG_SEND_FIFO = 0x55AAAA55;
				break;
			}
#endif
			case 0xAA5500F8:
				while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
				val = REG_RECV_FIFO;
				gba_sound_set_src(val);
				break;
			case 0xAA5500F9:
				/*while(*((vu32*)0x04000184) & (1 << 8));
					vals[0] = REG_RECV_FIFO;
				while(*((vu32*)0x04000184) & (1 << 8));
					vals[1] = REG_RECV_FIFO;
				while(*((vu32*)0x04000184) & (1 << 8));
					vals[2] = REG_RECV_FIFO;
				while(*((vu32*)0x04000184) & (1 << 8));
					vals[3] = REG_RECV_FIFO;
				gba_sound_fifo_write16((uint8_t*)&vals[0]);*/
				gba_sound_fifo_update();
				break;
			case 0xAA5500FA: //gb sound update
				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					int reg = val & 0xFF;
					int len = (val >> 8) & 0x7;
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					for (int i = 0; i < len; i++)
					{
						if (reg == 0x82)
						{
							gbs_setMixVolume(val & 3);
							gbas_updateVolume(val & 0xFF);
						}
						else if (reg == 0x83)
							gbas_updateMixConfig(val & 0xFF);
						else
							gbs_writeReg(reg, val & 0xFF);
						reg++;
						val >>= 8;
					}
					break;
				}
			case 0xAA5500FF://set master volume
				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;					
					if(val & 0x80000000)
						gba_sound_resync();
					int volume = val & 0x7F;
					REG_SOUNDCNT = (REG_SOUNDCNT & ~0x7F) | volume;
					break;
				}
			case 0xAA560000: //sio reinit
				sio_init();
				break;
			case 0xAA550100: //get rtc data
				{
					u8 dateTime[8];
					u8 cmd = READ_TIME_AND_DATE;
					rtc_doTransfer(&cmd, 1, dateTime, 7);
					cmd = READ_STATUS_REG1;
					rtc_doTransfer(&cmd, 1, &dateTime[7], 1);
					REG_SEND_FIFO = *(u32*)&dateTime[0];
					REG_SEND_FIFO = *(u32*)&dateTime[4];
					break;
				}
			case 0x040000A0:
				while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
				val = REG_RECV_FIFO;
				gba_sound_fifo_write(val);
				break;
			case 0x04000100:
				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					gbas_soundTimerUpdated(0, val >> 16, val & 0xFFFF);
					break;
				}
			case 0x04000102:
				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					gbas_soundTimerControlUpdated(0, val & 0xFFFF);
					break;
				}
			case 0x04000104:
				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					gbas_soundTimerUpdated(1, val >> 16, val & 0xFFFF);
					break;
				}
			case 0x04000106:

				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					gbas_soundTimerControlUpdated(1, val & 0xFFFF);
					break;
				}
			case 0x04000108:
			case 0x0400010A:
			case 0x0400010C:
			case 0x0400010E:
				{
					while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					break;
				}
			case 0x04000128:
				{
					while(REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					sio_writeReg16(0x128, val);
					break;
				}		
			case 0x0400012A:
				{
					while(REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					sio_writeReg16(0x12A, val);
					break;
				}
			case 0x04000134:
				{
					while(REG_FIFO_CNT & FIFO_CNT_EMPTY);
					val = REG_RECV_FIFO;
					sio_writeReg16(0x134, val);
					break;
				}
		}
	}
	return 0;
}
