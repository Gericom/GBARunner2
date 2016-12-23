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

volatile int soundStarted;

static void gba_sound_timer2_handler()
{
	soundBufferVirtualReadOffset += 256;
	if(soundBufferVirtualReadOffset > soundBufferVirtualWriteOffset + (SOUND_BUFFER_SIZE * 4))
	{
		irqDisable(IRQ_TIMER2);
		REG_TM[2].CNT_H = 0;
		soundStarted = 0;//resync
	}
}

void gba_sound_init()
{
	REG_SOUNDCNT = REG_SOUNDCNT_E | 0x7F;
	REG_SOUNDBIAS = 0x200;
	soundStarted = 0;
	soundBufferWriteOffset = 0;
	//soundBufferVirtualWriteOffset = 0;
	//soundBufferVirtualReadOffset = 0;
	//REG_TM[2].CNT_H = 0;
	//irqDisable(IRQ_TIMER2);
	//irqSet(IRQ_TIMER2, gba_sound_timer2_handler);
}

void gba_sound_notify_reset()
{
	if(!(*((vu32*)0x04000136) & 1))
	{
		soundStarted = 0;
		soundBufferWriteOffset = 0;
	}
	//int oldirq = enterCriticalSection();
	//old value
	u16 count = REG_TM[1].CNT_L; //in samples
	//reset
	REG_TM[0].CNT_H = 0;
	REG_TM[1].CNT_H = 0;
	REG_TM[0].CNT_L = TIMER_FREQ(/*10512);//*/13378);//10512);
	REG_TM[1].CNT_L = 0;
	REG_TM[1].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_CH;
	REG_TM[0].CNT_H = REG_TMXCNT_H_E;
	if(samplesPerBlock == 0)
		samplesPerBlock = (count == 0) ? 0 : count + 3;
	else
		samplesPerBlock = (3 * samplesPerBlock + ((count == 0) ? 0 : count + 3)) / 4;//(u32)(((u64)count * 598261ull + 298685ull) / 597370ull);
	//append the block to the ringbuffer
	if(samplesPerBlock == 0 || samplesPerBlock > SOUND_BUFFER_SIZE)
		return;
	if(SOUND_BUFFER_SIZE - soundBufferWriteOffset >= samplesPerBlock)
	{
		//while(dmaBusy(2));
		//dmaCopyWordsAsynch(2, (void*)0x23F8000, &soundBuffer[soundBufferWriteOffset], samplesPerBlock);
		memcpy(&soundBuffer[soundBufferWriteOffset], (void*)0x23F8000, samplesPerBlock);
	}
	else
	{
		//wrap around
		uint32_t left = SOUND_BUFFER_SIZE - soundBufferWriteOffset;
		//while(dmaBusy(2));
		//dmaCopyWordsAsynch(2, (void*)0x23F8000, &soundBuffer[soundBufferWriteOffset], left);
		//while(dmaBusy(3));
		//dmaCopyWordsAsynch(3, (void*)(0x23F8000 + left), &soundBuffer[0], samplesPerBlock - left);
		memcpy(&soundBuffer[soundBufferWriteOffset], (void*)0x23F8000, left);
		memcpy(&soundBuffer[0], (void*)(0x23F8000 + left), samplesPerBlock - left);
	}
	soundBufferWriteOffset += samplesPerBlock;
	if(soundBufferWriteOffset >= SOUND_BUFFER_SIZE)
		soundBufferWriteOffset -= SOUND_BUFFER_SIZE;
	//soundBufferVirtualWriteOffset += samplesPerBlock;
	if(!soundStarted && soundBufferWriteOffset >= (SOUND_BUFFER_SIZE / 8))
	{
		//REG_TM[2].CNT_H = 0;
		//soundBufferVirtualReadOffset = 0;
		//soundBufferVirtualWriteOffset = soundBufferWriteOffset;

		REG_SOUND[0].SAD = (u32)&soundBuffer[0];
		REG_SOUND[0].TMR = (u16)-1253;//-1594; //-1253
		REG_SOUND[0].PNT = 0;
		REG_SOUND[0].LEN = SOUND_BUFFER_SIZE >> 2;//396 * 10;
		REG_SOUND[0].CNT = 0;
	
		//REG_TM[2].CNT_L = TIMER_FREQ(13378);

		REG_SOUND[0].CNT = REG_SOUNDXCNT_E | REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) | REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) | REG_SOUNDXCNT_PAN(64) | REG_SOUNDXCNT_VOLUME(0x7F);//SOUND_CHANNEL_0_SETTINGS;
		soundStarted = 1;
		//REG_TM[2].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_256;
		//irqEnable(IRQ_TIMER2);
	}
	//leaveCriticalSection(oldirq);
}

void gba_sound_vblank()
{

}