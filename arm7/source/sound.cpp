#include <nds.h>
#include <string.h>
#include "timer.h"
#include "sound.h"

#define SOUND_BUFFER_SIZE	8192

s8 soundBuffer[SOUND_BUFFER_SIZE];

uint32_t soundBufferWriteOffset;

volatile uint32_t soundBufferVirtualWriteOffset;
volatile uint32_t soundBufferVirtualReadOffset;

uint32_t samplesPerBlock;

int sampleFreq;

volatile int soundStarted;

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

void gba_sound_init()
{
	REG_SOUNDCNT = REG_SOUNDCNT_E | 0x7F;
	REG_SOUNDBIAS = 0x200;
	soundStarted = 0;
	soundBufferWriteOffset = 0;
	sampleFreq = 0;
	//soundBufferVirtualWriteOffset = 0;
	//soundBufferVirtualReadOffset = 0;
	//REG_TM[2].CNT_H = 0;
	//irqDisable(IRQ_TIMER2);
	//irqSet(IRQ_TIMER2, gba_sound_timer2_handler);
}

static inline void gba_sound_resync()
{
	soundStarted = 0;
	soundBufferWriteOffset = 0;
}

void gba_sound_notify_reset()
{
	if(sampleFreq <= 0)
		return;
	if(!(*((vu32*)0x04000136) & 1))
		gba_sound_resync();
	//old value
	u16 count = REG_TM[1].CNT_L; //in samples
	//reset
	REG_TM[0].CNT_H = 0;
	REG_TM[1].CNT_H = 0;
	REG_TM[0].CNT_L = TIMER_FREQ(sampleFreq);///*10512);//*/13378);//10512);
	REG_TM[1].CNT_L = 0;
	REG_TM[1].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_CH;
	REG_TM[0].CNT_H = REG_TMXCNT_H_E;
	uint32_t newSamplesPerBlock = (count + 8) & ~0xF;
	if(newSamplesPerBlock > samplesPerBlock * 3)
		gba_sound_resync();
	//if(samplesPerBlock == 0)
		samplesPerBlock = newSamplesPerBlock;
	//else
	//	samplesPerBlock = /*(((3 * samplesPerBlock + (*/(count + 8) & ~0xF;//)) / 4) + 8) & ~0xF;//(u32)(((u64)count * 598261ull + 298685ull) / 597370ull);
	//append the block to the ringbuffer
	if(samplesPerBlock == 0 || samplesPerBlock > SOUND_BUFFER_SIZE)
	{
		gba_sound_resync();
		return;
	}
	if(SOUND_BUFFER_SIZE - soundBufferWriteOffset >= samplesPerBlock)
	{
		while(dmaBusy(2));
		dmaCopyWordsAsynch(2, (void*)0x23F8000, &soundBuffer[soundBufferWriteOffset], samplesPerBlock);
		//memcpy(&soundBuffer[soundBufferWriteOffset], (void*)0x23F8000, samplesPerBlock);
	}
	else
	{
		//wrap around
		uint32_t left = SOUND_BUFFER_SIZE - soundBufferWriteOffset;
		while(dmaBusy(2));
		dmaCopyWordsAsynch(2, (void*)0x23F8000, &soundBuffer[soundBufferWriteOffset], left);
		while(dmaBusy(3));
		dmaCopyWordsAsynch(3, (void*)(0x23F8000 + left), &soundBuffer[0], samplesPerBlock - left);
		//memcpy(&soundBuffer[soundBufferWriteOffset], (void*)0x23F8000, left);
		//memcpy(&soundBuffer[0], (void*)(0x23F8000 + left), samplesPerBlock - left);
	}
	soundBufferWriteOffset += samplesPerBlock;
	if(soundBufferWriteOffset >= SOUND_BUFFER_SIZE)
		soundBufferWriteOffset -= SOUND_BUFFER_SIZE;
	//soundBufferVirtualWriteOffset += samplesPerBlock;
	if(!soundStarted && soundBufferWriteOffset >= (SOUND_BUFFER_SIZE / 4))
	{
		//REG_TM[2].CNT_H = 0;
		//soundBufferVirtualReadOffset = 0;
		//soundBufferVirtualWriteOffset = soundBufferWriteOffset;

		REG_SOUND[0].SAD = (u32)&soundBuffer[0];
		REG_SOUND[0].TMR = (u16)(-(33513982/2)/sampleFreq);//(u16)-1253;//-1594; //-1253
		REG_SOUND[0].PNT = 0;
		REG_SOUND[0].LEN = SOUND_BUFFER_SIZE >> 2;//396 * 10;
		REG_SOUND[0].CNT = 0;
	
		//REG_TM[2].CNT_L = TIMER_FREQ(13378);

		REG_SOUND[0].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) | REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(64) | REG_SOUNDXCNT_VOLUME(0x7F);//SOUND_CHANNEL_0_SETTINGS;
		soundStarted = 1;
		//REG_TM[2].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_256;
		//irqEnable(IRQ_TIMER2);
	}
}

void gba_sound_vblank()
{

}

void gba_sound_timer_updated(uint16_t reloadVal)
{
	int freq = (-16 * 1024 * 1024) / ((int16_t)reloadVal);
	if(sampleFreq != freq)
	{
		sampleFreq = freq;
		gba_sound_resync();
	}
}