#ifndef __SOUND_H__
#define __SOUND_H__

typedef volatile struct
{
	vu32 CNT;
	vu32 SAD;
	vu16 TMR;
	vu16 PNT;
	vu32 LEN;
} sound_channel_t;

#define REG_SOUND		((sound_channel_t*)0x04000400)

#define REG_SOUNDXCNT_VOLUME(x)		(((x) & 0x7F) << 0)
#define REG_SOUNDXCNT_SHIFT(x)		(((x) & 3) << 8)
#define REG_SOUNDXCNT_HOLD			(1 << 15)
#define REG_SOUNDXCNT_PAN(x)		(((x) & 0x7F) << 16)
#define REG_SOUNDXCNT_DUTY(x)		(((x) & 7) << 24)
#define REG_SOUNDXCNT_REPEAT(x)		(((x) & 3) << 27)

#define REG_SOUNDXCNT_REPEAT_MANUAL		0
#define REG_SOUNDXCNT_REPEAT_LOOP		1
#define REG_SOUNDXCNT_REPEAT_ONE_SHOT	2

#define REG_SOUNDXCNT_FORMAT(x)		(((x) & 3) << 29)

#define REG_SOUNDXCNT_FORMAT_PCM8		0
#define REG_SOUNDXCNT_FORMAT_PCM16		1
#define REG_SOUNDXCNT_FORMAT_ADPCM		2
#define REG_SOUNDXCNT_FORMAT_PSG_NOISE	3

#define REG_SOUNDXCNT_E				(1 << 31)

#define REG_SOUNDCNT	(*((vu32*)0x04000500))

#define REG_SOUNDCNT_MIX_CH1	(1 << 12)
#define REG_SOUNDCNT_MIX_CH3	(1 << 13)
#define REG_SOUNDCNT_E			(1 << 15)

#define REG_SOUNDBIAS	(*((vu32*)0x04000504))

#if defined(USE_DSI_16MB) || defined(USE_3DS_32MB)
#define SOUNDEXCNT_NTR_DSP_RATIO_8_0	(8 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_7_1	(7 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_6_2	(6 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_5_3	(5 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_4_4	(4 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_3_5	(3 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_2_6	(2 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_1_7	(1 << 0)
#define SOUNDEXCNT_NTR_DSP_RATIO_0_8	(0 << 0)

#define SOUNDEXCNT_FREQ_32				(0 << 13)
#define SOUNDEXCNT_FREQ_47				(1 << 13)

#define SOUNDEXCNT_MUTE_SOMETHING		(1 << 14)

#define SOUNDEXCNT_ENABLE_SOMETHING		(1 << 15)

#define REG_SOUNDEXCNT	(*((vu32*)0x04004700))
#endif

struct dsound_channel_t
{
	u8 timer;
	u8 volume;
	u8 enables;
	u8 padding;

	int frequency;
	int curTimer;
	u32 curAddress;

	volatile bool started;
	int sampCounter;

	//for samplerate dithering
	int rateCounter;
	int rateDiffLo;
	int rateDiffHi;
	u32 rateTmrLo;
};

void gba_sound_init();
void gba_sound_notify_reset();
void gba_sound_vblank();
void gbas_soundTimerUpdated(int timer, u16 controlVal, u16 reloadVal);
void gbas_soundTimerControlUpdated(int timer, u16 controlVal);
void gba_sound_fifo_write(uint32_t samps);
void gba_sound_set_src(uint32_t address);
void gba_sound_fifo_write16(uint8_t* samps);
void gba_sound_resync();
void gba_sound_fifo_update();
void gbas_updateVolume(u8 vol);
void gbas_updateMixConfig(u8 mixConfig);

#endif