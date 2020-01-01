#include <nds.h>
#include <string.h>
#include "timer.h"
#include "../../common/fifo.h"
#include "../../common/sd_vram.h"
#include "lock.h"
#include "system.h"
#include "sound.h"

#define USE_ACTUAL_RATE
#define ACCESS_MAIN_MEM_DIRECT

#define SOUND_BUFFER_SIZE	8192

#define FIFO_BLOCK_SIZE	16

s8 soundBuffer[SOUND_BUFFER_SIZE];

static s8 sSoundFifoA[32];
static int sSoundFifoACount;

uint32_t soundBufferWriteOffset;

volatile uint32_t soundBufferVirtualWriteOffset;
volatile uint32_t soundBufferVirtualReadOffset;

u16 sTimerReloadVals[4];
u16 sTimerControlVals[4];

static dsound_channel_t sDSoundChannels[2];

/*static void gba_sound_timer2_handler()
{
	soundBufferVirtualReadOffset += 256;
	if(soundBufferVirtualReadOffset > soundBufferVirtualWriteOffset + (SOUND_BUFFER_SIZE * 4))
	{
		irqDisable(IRQ_TIMER2);
		REG_TM[2].CNT_H = 0;
		sDSoundChannels[0].started = 0;//resync
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
	if(sDSoundChannels[channel].frequency <= 0 || sDSoundChannels[channel].frequency >= 1000000)
	{
		volumeL = 0;
		volumeR = 0;	
	}
	int vol = sDSoundChannels[channel].volume == 1 ? 0x7F : 0x40;
	if((sDSoundChannels[channel].enables & 1) || (sDSoundChannels[channel].enables & 2))
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
	//volumeL = (sDSoundChannels[0].enables & 1) ? vol : 0;
	//volumeR = (sDSoundChannels[0].enables & 2) ? vol : 0;
}

void gba_sound_init()
{
	REG_SOUNDCNT = REG_SOUNDCNT_E | 0x7F;
	REG_SOUNDBIAS = 0x200;
#if defined(USE_DSI_16MB) || defined(USE_3DS_32MB)
	REG_SOUNDEXCNT = 0;
	REG_SOUNDEXCNT = SOUNDEXCNT_ENABLE_SOMETHING | SOUNDEXCNT_FREQ_47 | SOUNDEXCNT_NTR_DSP_RATIO_8_0;
#endif
	soundBufferWriteOffset = 0;
	vram_cd->sound_emu_work.req_read_ptr = 0;
	vram_cd->sound_emu_work.req_write_ptr = 0;
	vram_cd->sound_emu_work.resp_read_ptr = 0;
	vram_cd->sound_emu_work.resp_write_ptr = 0;
	memset(sDSoundChannels, 0, sizeof(sDSoundChannels));
	sDSoundChannels[0].volume = 1;
	sDSoundChannels[1].volume = 1;
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
		sDSoundChannels[0].started = false;
		soundBufferWriteOffset = 0;
		//sDSoundChannels[0].curAddress = 0;
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
	if (!sDSoundChannels[0].started && /*((sDSoundChannels[0].curAddress >> 24) == 2 ||*/ soundBufferWriteOffset >= (SOUND_BUFFER_SIZE / 8))//)
	{
		//REG_TM[2].CNT_H = 0;
		//soundBufferVirtualReadOffset = 0;
		//soundBufferVirtualWriteOffset = soundBufferWriteOffset;

		REG_SOUND[0].CNT = 0;
		REG_SOUND[1].CNT = 0;
		REG_SOUND[0].SAD = (u32)&soundBuffer[0];
		REG_SOUND[1].SAD = (u32)&soundBuffer[0];
		REG_SOUND[0].TMR = -sDSoundChannels[0].curTimer;//sTimerReloadVals[sDSoundChannels[0].timer]; //(u16)-1253;//-1594; //-1253
		REG_SOUND[1].TMR = -sDSoundChannels[0].curTimer;//sTimerReloadVals[sDSoundChannels[0].timer]; //(u16)-1253;//-1594; //-1253
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
		sDSoundChannels[0].started = true;
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
	if (sDSoundChannels[0].curAddress == 0)
		return;
	if(sDSoundChannels[0].rateDiffLo != 0)
	{
		if(sDSoundChannels[0].rateCounter + sDSoundChannels[0].rateDiffHi >= 0)
		{
			sDSoundChannels[0].curTimer = sDSoundChannels[0].rateTmrLo + 1;
			REG_TM[3].CNT_L = -sDSoundChannels[0].curTimer << 1;
			REG_SOUND[0].TMR = -sDSoundChannels[0].curTimer;
			REG_SOUND[1].TMR = -sDSoundChannels[0].curTimer;
			sDSoundChannels[0].rateCounter += sDSoundChannels[0].rateDiffHi;
		}
		else
		{
			sDSoundChannels[0].curTimer = sDSoundChannels[0].rateTmrLo;
			REG_TM[3].CNT_L = -sDSoundChannels[0].curTimer << 1;
			REG_SOUND[0].TMR = -sDSoundChannels[0].curTimer;
			REG_SOUND[1].TMR = -sDSoundChannels[0].curTimer;
			sDSoundChannels[0].rateCounter += sDSoundChannels[0].rateDiffLo;
		}
	}
	if (sDSoundChannels[0].sampCounter == 0) //(FIFO_BLOCK_SIZE - 1))
	{
#ifdef ACCESS_MAIN_MEM_DIRECT
		if((sDSoundChannels[0].curAddress >> 24) == 2)
		{
			//data is in main memory, directly access it
			//gba_sound_fifo_write16((u8*)sDSoundChannels[0].curAddress);
			REG_SOUND[0].SAD = sDSoundChannels[0].curAddress;
			REG_SOUND[1].SAD = sDSoundChannels[0].curAddress;
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
				vram_cd->sound_emu_work.req_queue[vram_cd->sound_emu_work.req_write_ptr] = sDSoundChannels[0].curAddress;
				vram_cd->sound_emu_work.req_write_ptr = (vram_cd->sound_emu_work.req_write_ptr + 1) & (SOUND_EMU_QUEUE_LEN - 1);
			}
			else
			{
				for (int i = 0; i < FIFO_BLOCK_SIZE; i++)
					soundBuffer[(soundBufferWriteOffset + i) & (SOUND_BUFFER_SIZE - 1)] = soundBuffer[(soundBufferWriteOffset - FIFO_BLOCK_SIZE + i) & (SOUND_BUFFER_SIZE - 1)];
				soundBufferWriteOffset = (soundBufferWriteOffset + FIFO_BLOCK_SIZE) & (SOUND_BUFFER_SIZE - 1);
				gba_sound_update_ds_channels();
				/*vram_cd->sound_emu_work.req_queue[vram_cd->sound_emu_work.req_write_ptr] = sDSoundChannels[0].curAddress;
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
			REG_IPC_SYNC |= IPC_SYNC_IRQ_REQUEST;
		}
		sDSoundChannels[0].curAddress += FIFO_BLOCK_SIZE; //16;		
	}
	sDSoundChannels[0].sampCounter++;
	if (sDSoundChannels[0].sampCounter == FIFO_BLOCK_SIZE)
		sDSoundChannels[0].sampCounter = 0;
}

static void updateChannelVolume(int channel)
{
	if(!sDSoundChannels[channel].started)
		return;
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
	if (sTimerControlVals[sDSoundChannels[0].timer] & 0x80)
		freq = (-16 * 1024 * 1024) / ((s16)sTimerReloadVals[sDSoundChannels[0].timer]);
	if (sDSoundChannels[0].frequency != freq)
	{
		REG_TM[3].CNT_H = 0;
		REG_IF = IRQ_TIMER(3);

		s64 exactFreq = ((s64)(-16 * 1024 * 1024) << 16) / ((s16)sTimerReloadVals[sDSoundChannels[0].timer]);

		sDSoundChannels[0].frequency = freq;
#ifdef USE_ACTUAL_RATE
		u32 tmrLo = (((s64)(SYS_CLOCK_FREQ >> 1) << 16) / exactFreq) << 16;
		u32 tmrHi = tmrLo + (1 << 16);
		u32 tmrTgt = ((s64)(SYS_CLOCK_FREQ >> 1) << 32) / exactFreq;
#else
		u32 tmrLo = (-(s16)sTimerReloadVals[sDSoundChannels[0].timer]) << 16;
		u32 tmrHi = (-(s16)sTimerReloadVals[sDSoundChannels[0].timer]) << 16;
		u32 tmrTgt = (-(s16)sTimerReloadVals[sDSoundChannels[0].timer]) << 16;
#endif
		sDSoundChannels[0].rateDiffLo = tmrTgt - tmrLo;
		sDSoundChannels[0].rateDiffHi = tmrTgt - tmrHi;

		sDSoundChannels[0].rateTmrLo = tmrLo >> 16;
		sDSoundChannels[0].curTimer = tmrLo >> 16;//(((SYS_CLOCK_FREQ + ((sDSoundChannels[0].frequency + 1) >> 1)) / sDSoundChannels[0].frequency) + 1) >> 1;

		if (sDSoundChannels[0].frequency > 0 && sDSoundChannels[0].frequency < 1000000)
		{
			sDSoundChannels[0].rateCounter = 0;
			REG_TM[3].CNT_L = -sDSoundChannels[0].curTimer << 1;//((s16)sTimerReloadVals[sDSoundChannels[0].timer]) << 1;
			REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I;
			REG_IE |= IRQ_TIMER(3);
			REG_SOUND[0].TMR = -sDSoundChannels[0].curTimer;//sTimerReloadVals[sDSoundChannels[0].timer];
			REG_SOUND[1].TMR = -sDSoundChannels[0].curTimer;//sTimerReloadVals[sDSoundChannels[0].timer];
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
	if (sDSoundChannels[0].timer == timer)
		gbas_updateChannelATimer();
}

void gbas_soundTimerControlUpdated(int timer, u16 controlVal)
{
	sTimerControlVals[timer] = controlVal;
	if (sDSoundChannels[0].timer == timer)
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
	if(sDSoundChannels[0].started && (sDSoundChannels[0].curAddress == address - 16 || sDSoundChannels[0].curAddress == address || sDSoundChannels[0].curAddress == address + 16))
		return;
	int oldIrq = enterCriticalSection();
#ifdef ACCESS_MAIN_MEM_DIRECT
	if(((sDSoundChannels[0].curAddress >> 24) != 2 && (address >> 24) == 2) || ((sDSoundChannels[0].curAddress >> 24) == 2 && (address >> 24) != 2))
		gba_sound_resync();
#endif
	sDSoundChannels[0].curAddress = address;
#ifdef ACCESS_MAIN_MEM_DIRECT
	if((address >> 24) != 2)
#endif
	{
		REG_TM[3].CNT_H = 0;
		REG_IF = IRQ_TIMER(3);
		sDSoundChannels[0].sampCounter = 0;
	}
	//timer3_overflow_irq();
	if (sDSoundChannels[0].frequency != 0)
	{
#ifdef ACCESS_MAIN_MEM_DIRECT
		if((address >> 24) == 2)
		{
			if(!sDSoundChannels[0].started)
			{
				REG_TM[3].CNT_H = 0;
				REG_IF = IRQ_TIMER(3);
				REG_SOUND[0].CNT = 0;
				REG_SOUND[1].CNT = 0;
				REG_SOUND[0].SAD = address;
				REG_SOUND[1].SAD = address;
				REG_SOUND[0].TMR = -sDSoundChannels[0].curTimer;
				REG_SOUND[1].TMR = -sDSoundChannels[0].curTimer;
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
				sDSoundChannels[0].started = true;
				sDSoundChannels[0].curAddress = address + 16;
				sDSoundChannels[0].rateCounter = 0;
				sDSoundChannels[0].sampCounter = 0;

				REG_TM[3].CNT_L = -sDSoundChannels[0].curTimer << 1;
				REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I;
				REG_IE |= IRQ_TIMER(3);
			}
		}
		else
#endif
		{
			sDSoundChannels[0].rateCounter = 0;
			REG_TM[3].CNT_L = -sDSoundChannels[0].curTimer << 1;
			REG_TM[3].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I;
			REG_IE |= IRQ_TIMER(3);
		}
	}
	leaveCriticalSection(oldIrq);
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
	sDSoundChannels[0].volume = (vol >> 2) & 1;
	sDSoundChannels[1].volume = (vol >> 3) & 1;
	updateChannelVolume(0);
	updateChannelVolume(1);
}

void gbas_updateMixConfig(u8 mixConfig)
{
	int newATimer = (mixConfig >> 2) & 1;
	if (sDSoundChannels[0].timer != newATimer)
	{
		sDSoundChannels[0].timer = newATimer;
		gbas_updateChannelATimer();
	}
	int newBTimer = (mixConfig >> 6) & 1;
	if (sDSoundChannels[1].timer != newBTimer)
	{
		sDSoundChannels[1].timer = newBTimer;
		//gbas_updateChannelBTimer();
	}
	sDSoundChannels[0].enables = mixConfig & 3;
	sDSoundChannels[1].enables = (mixConfig >> 4) & 3;
	updateChannelVolume(0);
	updateChannelVolume(1);
}
