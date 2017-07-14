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

void gba_sound_init();
void gba_sound_notify_reset();
void gba_sound_vblank();
void gba_sound_timer_updated(uint16_t reloadVal);
void gba_sound_fifo_write(uint32_t samps);
void gba_sound_set_src(uint32_t address);
void gba_sound_fifo_write16(uint8_t* samps);
void gba_sound_resync();
void gba_sound_fifo_update();

#endif