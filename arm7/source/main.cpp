#include <nds.h>
#include <string.h>
#include "timer.h"
#include "sound.h"
#include "gbsound.h"
#include "save.h"
#include "dldi_handler.h"
#include "../../common/fifo.h"
#include "../../common/common_defs.s"
#include "rtcom/rtcom.h"

#ifdef USE_3DS_32MB
enum RtcomState
{
	RTCOM_STATE_OFF,
	RTCOM_STATE_EXEC_UCODE3_ACK
};

static bool sRtcomInitialized = false;
static RtcomState sRtcomState = RTCOM_STATE_OFF;
static s16 sRtcomGyroZTmp;
static bool sGyroEnabled = false;
//static volatile bool sSampleGyro = false;


extern "C" void rtcomIrq()
{
	switch(sRtcomState)
	{
		case RTCOM_STATE_EXEC_UCODE3_ACK:
		{
			//sSampleGyro = true;
			s16 gyro0 = rtcom_getData();
			rtcom_requestNext(4);
			vram_cd->gyroZ = gyro0 | (rtcom_getData() << 8);
			rtcom_requestKill();
			if(sGyroEnabled)
			{
				sRtcomState = RTCOM_STATE_EXEC_UCODE3_ACK;
				rtcom_requestAsync(RTCOM_REQ_EXECUTE_UCODE, 3);
			}
			else
			{
				REG_IE &= ~IRQ_NETWORK;
				sRtcomState = RTCOM_STATE_OFF;
				rtcom_endComm();
			}
			break;
		}
	}
}

// static void sampleGyro()
// {
// 	sSampleGyro = false;
// 	REG_IE &= ~IRQ_NETWORK;
// 	s16 gyro0 = rtcom_getData();
// 	rtcom_requestNext(4);
// 	vram_cd->gyroZ = gyro0 | (rtcom_getData() << 8);
// 	rtcom_requestKill();
// 	if(sGyroEnabled)
// 	{
// 		REG_IE |= IRQ_NETWORK;
// 		sRtcomState = RTCOM_STATE_IDLE;
// 		rtcom_requestAsync(RTCOM_STAT_DONE);
// 	}
// 	else
// 	{
// 		sRtcomState = RTCOM_STATE_OFF;
// 		rtcom_endComm();
// 	}
// }
#endif

// extern "C" void vblankHandler()
// {
// #ifdef USE_3DS_32MB
// 	int savedIrq = enterCriticalSection();
// 	{
// 		rtcom_beginComm();
// 		updateGyro();
// 		rtcom_requestKill();
// 		rtcom_requestAsync(RTCOM_STAT_DONE);
// 		rtcom_endComm();
// 	}
// 	leaveCriticalSection(savedIrq);
// #endif
// }

extern "C" void my_irq_handler();

#ifdef USE_3DS_32MB
#endif

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
#ifdef USE_3DS_32MB
	while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
	void* uc11Addr = (void*)REG_RECV_FIFO;
	while (REG_FIFO_CNT & FIFO_CNT_EMPTY);
	u32 uc11Size = REG_RECV_FIFO;
	int savedIrq = enterCriticalSection();
	{
		rtcom_beginComm();
		rtcom_requestAsync(1);
		rtcom_waitStatus(RTCOM_STAT_DONE);
		rtcom_uploadUCode(uc11Addr, uc11Size);
		rtcom_requestKill();
		if(rtcom_executeUCode(0xFF)) //mcu init
			sRtcomInitialized = true;
		rtcom_requestKill();
		rtcom_requestAsync(RTCOM_STAT_DONE);
		rtcom_endComm();
	}
	leaveCriticalSection(savedIrq);

	sGyroEnabled = false;
	sRtcomState = RTCOM_STATE_OFF;
#endif

	//int oldirq = enterCriticalSection();
	//irqSet(IRQ_HBLANK, hblank_handler);

	gba_sound_init();
	gbs_init();
	gba_save_init();


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
			if (!(*((vu32*)0x04000136) & 1))
				gba_sound_resync();

// #ifdef USE_3DS_32MB
// 			if(sSampleGyro)
// 				sampleGyro();
// #endif
		}

// #ifdef USE_3DS_32MB
// 		if(sSampleGyro)
// 			sampleGyro();
// #endif

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
			case 0xAA550100: //get rtc data
				{
					u8 dateTime[8];
					u8 cmd = READ_TIME_AND_DATE;
					rtcTransaction(&cmd, 1, dateTime, 7);
					cmd = READ_STATUS_REG1;
					rtcTransaction(&cmd, 1, &dateTime[7], 1);
					REG_SEND_FIFO = *(u32*)&dateTime[0];
					REG_SEND_FIFO = *(u32*)&dateTime[4];
					break;
				}
#ifdef USE_3DS_32MB
			case 0xAA550101: //enable gyro sampling
				if(sRtcomState == RTCOM_STATE_OFF)
				{
					sGyroEnabled = true;
					rtcom_beginComm();
					REG_IE |= IRQ_NETWORK;
					sRtcomState = RTCOM_STATE_EXEC_UCODE3_ACK;
					rtcom_requestAsync(RTCOM_REQ_EXECUTE_UCODE, 3);
				}
				break;
			case 0xAA550102: //disable gyro sampling
				sGyroEnabled = false;
				break;
#endif
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
				//case 0xC5://fifo_stop_sound_command
				//REG_SOUND[0].CNT = SOUND_CHANNEL_0_SETTINGS;
				//	break;
		}
	}
	return 0;
}
