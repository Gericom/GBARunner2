#include <nds.h>
#include <string.h>
#include "timer.h"
#include "../../common/fifo.h"
#include "../../common/sd_vram.h"
#include "lock.h"
#include "sound.h"

#define USE_ACTUAL_RATE
#define ACCESS_MAIN_MEM_DIRECT

#define SOUND_BUFFER_SIZE	8192

#define FIFO_BLOCK_SIZE	16

s8 soundBuffer[SOUND_BUFFER_SIZE];

uint32_t soundBufferWriteOffset;

volatile uint32_t soundBufferVirtualWriteOffset;
volatile uint32_t soundBufferVirtualReadOffset;

uint32_t samplesPerBlock;

int sampleFreq;
int sampleTmr;

volatile int soundStarted;

uint32_t srcAddress = 0;

u8 sChannelATimer = 0;
u8 sChannelAVolume = 1;
u8 sChannelAEnables = 0;

u16 sTimerReloadVals[4];
u16 sTimerControlVals[4];

static int sampcnter = 0;
static s32 sRateCounterA = 0;
static s32 sRateDiffLo = 0;
static s32 sRateDiffHi = 0;
static u32 sRateTmrLo = 0;
static bool sRateIsHi = false;

/*static void gba_sound_timer2_handler()
{
	soundBufferVirtualReadOffset += 256;
	if(soundBufferVirtualReadOffset > soundBufferVirtualWriteOffset + (SOUND_BUFFER_SIZE * 4))
	{
		irqDisable(IRQ_TIMER2);
		REG_TM[2].CNT_H = 0;
		soundStarted = 0;//resync
	}
}*/

static void calcChannelVolume(int channel, int& volumeL, int& volumeR)
{
	if(channel == 1)
	{
		volumeL = 0;
		volumeR = 0;
		return;
	}
	if(sampleFreq <= 0 || sampleFreq >= 1000000)
	{
		volumeL = 0;
		volumeR = 0;	
	}
	int vol = sChannelAVolume == 1 ? 0x7F : 0x40;
	if((sChannelAEnables & 1) || (sChannelAEnables & 2))
	{
		volumeL = vol;
		volumeR = vol;
	}
	else
	{
		volumeL = 0;
		volumeR = 0;
	}
	//todo: enable this when both channels are implemented
	//volumeL = (sChannelAEnables & 1) ? vol : 0;
	//volumeR = (sChannelAEnables & 2) ? vol : 0;
}

void gba_sound_init()
{
	REG_SOUNDCNT = REG_SOUNDCNT_E | 0x7F;
	REG_SOUNDBIAS = 0x200;
	soundStarted = 0;
	soundBufferWriteOffset = 0;
	sampleFreq = 0;
	srcAddress = 0;
	vram_cd->sound_emu_work.req_read_ptr = 0;
	vram_cd->sound_emu_work.req_write_ptr = 0;
	vram_cd->sound_emu_work.resp_read_ptr = 0;
	vram_cd->sound_emu_work.resp_write_ptr = 0;
	//soundBufferVirtualWriteOffset = 0;
	//soundBufferVirtualReadOffset = 0;
	//REG_TM[2].CNT_H = 0;
	//irqDisable(IRQ_TIMER2);
	//irqSet(IRQ_TIMER2, gba_sound_timer2_handler);
}

void gba_sound_resync()
{
	int oldIrq = enterCriticalSection();
	{
		soundStarted = 0;
		soundBufferWriteOffset = 0;
		//srcAddress = 0;
		vram_cd->sound_emu_work.req_write_ptr = vram_cd->sound_emu_work.req_read_ptr;
		vram_cd->sound_emu_work.resp_read_ptr = vram_cd->sound_emu_work.resp_write_ptr;
		//REG_TM[3].CNT_H = 0;
		REG_SOUND[0].CNT = 0;
		REG_SOUND[1].CNT = 0;
	}
	leaveCriticalSection(oldIrq);
}

static void gba_sound_update_ds_channels()
{
	if (!soundStarted && /*((srcAddress >> 24) == 2 ||*/ soundBufferWriteOffset >= (SOUND_BUFFER_SIZE / 8))//)
	{
		//REG_TM[2].CNT_H = 0;
		//soundBufferVirtualReadOffset = 0;
		//soundBufferVirtualWriteOffset = soundBufferWriteOffset;

		REG_SOUND[0].CNT = 0;
		REG_SOUND[1].CNT = 0;
		REG_SOUND[0].SAD = (u32)&soundBuffer[0];
		REG_SOUND[1].SAD = (u32)&soundBuffer[0];
		REG_SOUND[0].TMR = -sampleTmr;//sTimerReloadVals[sChannelATimer]; //(u16)-1253;//-1594; //-1253
		REG_SOUND[1].TMR = -sampleTmr;//sTimerReloadVals[sChannelATimer]; //(u16)-1253;//-1594; //-1253
		REG_SOUND[0].PNT = 0;
		REG_SOUND[1].PNT = 0;
		REG_SOUND[0].LEN = SOUND_BUFFER_SIZE >> 2; //396 * 10;
		REG_SOUND[1].LEN = SOUND_BUFFER_SIZE >> 2; //396 * 10;

		//REG_TM[2].CNT_L = TIMER_FREQ(13378);
		int volumeL, volumeR;
		calcChannelVolume(0, volumeL, volumeR);
		REG_SOUND[0].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
			REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(0) | REG_SOUNDXCNT_VOLUME(volumeL);
		//SOUND_CHANNEL_0_SETTINGS;
		REG_SOUND[1].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
			REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(127) | REG_SOUNDXCNT_VOLUME(volumeR);
		//SOUND_CHANNEL_0_SETTINGS;
		soundStarted = 1;
		//REG_TM[2].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_256;
		//irqEnable(IRQ_TIMER2);
	}
}

void gba_sound_notify_reset()
{

}

void gba_sound_vblank()
{
}

void gba_sound_fifo_update()
{
	while (vram_cd->sound_emu_work.resp_read_ptr != vram_cd->sound_emu_work.resp_write_ptr)
	{
		gba_sound_fifo_write16((u8*)&vram_cd->sound_emu_work.resp_queue[vram_cd->sound_emu_work.resp_read_ptr][0]);
		vram_cd->sound_emu_work.resp_read_ptr = (vram_cd->sound_emu_work.resp_read_ptr + 1) & (SOUND_EMU_QUEUE_LEN - 1);
	}
	gba_sound_update_ds_channels();
}

extern "C" void timer3_overflow_irq()
{
	if (srcAddress == 0)
		return;
	if(sRateDiffLo != 0)
	{
		if(sRateCounterA + sRateDiffHi >= 0)
		{
			sampleTmr = sRateTmrLo + 1;
			REG_TM[3].CNT_L = -sampleTmr << 1;
			REG_SOUND[0].TMR = -sampleTmr;
			REG_SOUND[1].TMR = -sampleTmr;
			sRateCounterA += sRateDiffHi;
		}
		else
		{
			sampleTmr = sRateTmrLo;
			REG_TM[3].CNT_L = -sampleTmr << 1;
			REG_SOUND[0].TMR = -sampleTmr;
			REG_SOUND[1].TMR = -sampleTmr;
			sRateCounterA += sRateDiffLo;
		}
	}
	if (sampcnter == 0) //(FIFO_BLOCK_SIZE - 1))
	{
#ifdef ACCESS_MAIN_MEM_DIRECT
		if((srcAddress >> 24) == 2)
		{
			//data is in main memory, directly access it
			//gba_sound_fifo_write16((u8*)srcAddress);
			REG_SOUND[0].SAD = srcAddress;
			REG_SOUND[1].SAD = srcAddress;
			//todo: still invoke an arm9 irq if games use dma irq
		}
		else
#endif
		{		
			gba_sound_fifo_update();
			int writeLength = vram_cd->sound_emu_work.req_read_ptr - vram_cd->sound_emu_work.req_write_ptr - 1;
			if (writeLength < 0)
				writeLength += SOUND_EMU_QUEUE_LEN;
			if (writeLength != 0)
			{
				vram_cd->sound_emu_work.req_queue[vram_cd->sound_emu_work.req_write_ptr] = srcAddress;
				vram_cd->sound_emu_work.req_write_ptr = (vram_cd->sound_emu_work.req_write_ptr + 1) & (SOUND_EMU_QUEUE_LEN - 1);
			}
			else
			{
				for (int i = 0; i < FIFO_BLOCK_SIZE; i++)
					soundBuffer[(soundBufferWriteOffset + i) & (SOUND_BUFFER_SIZE - 1)] = soundBuffer[(soundBufferWriteOffset - FIFO_BLOCK_SIZE + i) & (SOUND_BUFFER_SIZE - 1)];
				soundBufferWriteOffset = (soundBufferWriteOffset + FIFO_BLOCK_SIZE) & (SOUND_BUFFER_SIZE - 1);
				gba_sound_update_ds_channels();
				/*vram_cd->sound_emu_work.req_queue[vram_cd->sound_emu_work.req_write_ptr] = srcAddress;
				vram_cd->sound_emu_work.req_read_ptr++;
				if (vram_cd->sound_emu_work.req_read_ptr >= SOUND_EMU_QUEUE_LEN)
					vram_cd->sound_emu_work.req_read_ptr -= SOUND_EMU_QUEUE_LEN;
				vram_cd->sound_emu_work.req_write_ptr++;
				if (vram_cd->sound_emu_work.req_write_ptr >= SOUND_EMU_QUEUE_LEN)
					vram_cd->sound_emu_work.req_write_ptr -= SOUND_EMU_QUEUE_LEN;
				for (int i = 0; i < FIFO_BLOCK_SIZE; i++)
					soundBuffer[(soundBufferWriteOffset + i) % SOUND_BUFFER_SIZE] = soundBuffer[(soundBufferWriteOffset -
						FIFO_BLOCK_SIZE + i) % SOUND_BUFFER_SIZE];
				soundBufferWriteOffset += FIFO_BLOCK_SIZE;
				if (soundBufferWriteOffset >= SOUND_BUFFER_SIZE)
					soundBufferWriteOffset -= SOUND_BUFFER_SIZE;
				gba_sound_update_ds_channels();*/
			}
			//invoke an irq on arm9
			*((vu32*)0x04000180) |= (1 << 13);
		}
		srcAddress += FIFO_BLOCK_SIZE; //16;
	}
	sampcnter++;
	if (sampcnter == FIFO_BLOCK_SIZE)
		sampcnter = 0;
}

static void updateChannelVolume(int channel)
{
	if(channel == 0)
	{
		int volumeL, volumeR;
		calcChannelVolume(0, volumeL, volumeR);
		REG_SOUND[0].CNT = (REG_SOUND[0].CNT & ~REG_SOUNDXCNT_VOLUME(0x7F)) | REG_SOUNDXCNT_VOLUME(volumeL);
		REG_SOUND[1].CNT = (REG_SOUND[1].CNT & ~REG_SOUNDXCNT_VOLUME(0x7F)) | REG_SOUNDXCNT_VOLUME(volumeR);
	}
}

void gbas_updateChannelATimer()
{
	int freq = 0;
	if (sTimerControlVals[sChannelATimer] & 0x80)
		freq = (-16 * 1024 * 1024) / ((int16_t)sTimerReloadVals[sChannelATimer]);
	if (sampleFreq != freq)
	{
		REG_TM[3].CNT_H = 0;

		sampleFreq = freq;
#ifdef USE_ACTUAL_RATE
		u32 tmrLo = ((33513982 >> 1) / sampleFreq) << 16;
		u32 tmrHi = tmrLo + (1 << 16);
		u32 tmrTgt = ((33513982 >> 1) << 16) / sampleFreq;
#else
		u32 tmrLo = (-(int16_t)sTimerReloadVals[sChannelATimer]) << 16;
		u32 tmrHi = (-(int16_t)sTimerReloadVals[sChannelATimer]) << 16;
		u32 tmrTgt = (-(int16_t)sTimerReloadVals[sChannelATimer]) << 16;
#endif
		sRateDiffLo = tmrTgt - tmrLo;
		sRateDiffHi = tmrTgt - tmrHi;

		sRateTmrLo = tmrLo >> 16;
		sampleTmr = tmrLo >> 16;//(((33513982 + ((sampleFreq + 1) >> 1)) / sampleFreq) + 1) >> 1;

		if (sampleFreq > 0 && sampleFreq < 1000000)
		{
			sRateCounterA = 0;
			sRateIsHi = false;
			REG_TM[3].CNT_L = -sampleTmr << 1;//((s16)sTimerReloadVals[sChannelATimer]) << 1;
			REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I;
			REG_IE |= (1 << 6);
			REG_SOUND[0].TMR = -sampleTmr;//sTimerReloadVals[sChannelATimer];
			REG_SOUND[1].TMR = -sampleTmr;//sTimerReloadVals[sChannelATimer];
			int volumeL, volumeR;
			calcChannelVolume(0, volumeL, volumeR);
			REG_SOUND[0].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(0) | REG_SOUNDXCNT_VOLUME(volumeL);
			REG_SOUND[1].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(127) | REG_SOUNDXCNT_VOLUME(volumeR);
		}
		else
		{
			REG_SOUND[0].TMR = 0;
			REG_SOUND[1].TMR = 0;
			REG_SOUND[0].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(0) | REG_SOUNDXCNT_VOLUME(0);
			REG_SOUND[1].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(127) | REG_SOUNDXCNT_VOLUME(0);
		}
	}
}

void gbas_soundTimerUpdated(int timer, u16 controlVal, u16 reloadVal)
{
	sTimerControlVals[timer] = controlVal;
	sTimerReloadVals[timer] = reloadVal;
	if (sChannelATimer == timer)
		gbas_updateChannelATimer();
}

void gbas_soundTimerControlUpdated(int timer, u16 controlVal)
{
	sTimerControlVals[timer] = controlVal;
	if (sChannelATimer == timer)
		gbas_updateChannelATimer();
}

void gba_sound_fifo_write(uint32_t samps)
{
	soundBuffer[(soundBufferWriteOffset) % SOUND_BUFFER_SIZE] = samps & 0xFF;
	soundBuffer[(soundBufferWriteOffset + 1) % SOUND_BUFFER_SIZE] = (samps >> 8) & 0xFF;
	soundBuffer[(soundBufferWriteOffset + 2) % SOUND_BUFFER_SIZE] = (samps >> 16) & 0xFF;
	soundBuffer[(soundBufferWriteOffset + 3) % SOUND_BUFFER_SIZE] = (samps >> 24) & 0xFF;
	soundBufferWriteOffset += 4;
	if (soundBufferWriteOffset >= SOUND_BUFFER_SIZE)
		soundBufferWriteOffset -= SOUND_BUFFER_SIZE;
	gba_sound_update_ds_channels();
}

void gba_sound_set_src(uint32_t address)
{
	if(soundStarted && (srcAddress == address - 16 || srcAddress == address || srcAddress == address + 16))
		return;
	u32 oldAddr = srcAddress;
#ifdef ACCESS_MAIN_MEM_DIRECT
	if(((srcAddress >> 24) != 2 && (address >> 24) == 2) || ((srcAddress >> 24) == 2 && (address >> 24) != 2))
		gba_sound_resync();
#endif
	srcAddress = address;
#ifdef ACCESS_MAIN_MEM_DIRECT
	if((address >> 24) != 2)
#endif
	{
		REG_TM[3].CNT_H = 0;
		sampcnter = 0;
	}
	//timer3_overflow_irq();
	if (sampleFreq != 0)
	{
#ifdef ACCESS_MAIN_MEM_DIRECT
		if((address >> 24) == 2)
		{
			if(!soundStarted)
			{
				REG_TM[3].CNT_H = 0;
				REG_SOUND[0].CNT = 0;
				REG_SOUND[1].CNT = 0;
				REG_SOUND[0].SAD = address;
				REG_SOUND[1].SAD = address;
				REG_SOUND[0].TMR = -sampleTmr;
				REG_SOUND[1].TMR = -sampleTmr;
				REG_SOUND[0].PNT = 0;
				REG_SOUND[1].PNT = 0;
				REG_SOUND[0].LEN = 16 >> 2;
				REG_SOUND[1].LEN = 16 >> 2;

				int volumeL, volumeR;
				calcChannelVolume(0, volumeL, volumeR);
				REG_SOUND[0].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(0) | REG_SOUNDXCNT_VOLUME(volumeL);
				REG_SOUND[1].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(127) | REG_SOUNDXCNT_VOLUME(volumeR);
				soundStarted = 1;
				srcAddress = address;// + 16;
				sRateCounterA = 0;
				sampcnter = 0;

				REG_TM[3].CNT_L = -sampleTmr << 1;//((s16)sTimerReloadVals[sChannelATimer]) << 1; // * FIFO_BLOCK_SIZE;//* 64 / FIFO_BLOCK_SIZE);//16);
				REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I; // | REG_TMXCNT_H_PS_64;
				REG_IE |= (1 << 6);
			}
			else
			{			
				//srcAddress = address + 16;
				//REG_SOUND[0].SAD = address;
				//REG_SOUND[1].SAD = address;
			}
		}
		else
#endif
		{
			sRateCounterA = 0;
			REG_TM[3].CNT_L = -sampleTmr << 1;//((s16)sTimerReloadVals[sChannelATimer]) << 1; // * FIFO_BLOCK_SIZE;//* 64 / FIFO_BLOCK_SIZE);//16);
			REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I; // | REG_TMXCNT_H_PS_64;
			REG_IE |= (1 << 6);
		}
	}
}

void gba_sound_fifo_write16(uint8_t* samps)
{
	for (int i = 0; i < FIFO_BLOCK_SIZE; i++)
		soundBuffer[(soundBufferWriteOffset + i) % SOUND_BUFFER_SIZE] = samps[i];
	soundBufferWriteOffset += FIFO_BLOCK_SIZE;
	if (soundBufferWriteOffset >= SOUND_BUFFER_SIZE)
		soundBufferWriteOffset -= SOUND_BUFFER_SIZE;
	gba_sound_update_ds_channels();
}

void gbas_updateVolume(u8 vol)
{
	sChannelAVolume = (vol >> 2) & 1;
	if(soundStarted)
	{
		updateChannelVolume(0);
		updateChannelVolume(1);
	}
}

void gbas_updateMixConfig(u8 mixConfig)
{
	int newATimer = (mixConfig >> 2) & 1;
	if (sChannelATimer != newATimer)
	{
		sChannelATimer = newATimer;
		gbas_updateChannelATimer();
	}
	sChannelAEnables = mixConfig & 3;
	updateChannelVolume(0);
	updateChannelVolume(1);
}
