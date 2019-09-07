#include <nds.h>
#include <string.h>
#include "../../common/sd_vram.h"
#include "timer.h"
#include "sound.h"
#include "gbsound.h"

#define GB_CHANNEL_1_HW_L	8
#define GB_CHANNEL_1_HW_R	9
#define GB_CHANNEL_2_HW_L	10
#define GB_CHANNEL_2_HW_R	11
#define GB_CHANNEL_3_HW_L_0	6
#define GB_CHANNEL_3_HW_R_0	7
#define GB_CHANNEL_3_HW_L_1	12
#define GB_CHANNEL_3_HW_R_1	13
#define GB_CHANNEL_4_HW_L	14
#define GB_CHANNEL_4_HW_R	15

//information sources:
//- http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
//- http://belogic.com/gba/
//- https://github.com/Drenn1/GameYob
//- https://problemkaputt.de/gbatek.htm
//- https://github.com/mgba-emu/mgba

static int sFrameSeqStep;

static int  sChannelLength[4];
static int  sChannelLengthCounter[4];
static bool sChannelUseLen[4];

static int sChannelFreq[4];
static int sChannelVolume[4];
static int sChannelEnvDir[4];
static int sChannelEnvSweep[4];
static int sChannelVolumeTimer[4];

static int sChannel1Duty;
static int sChannel2Duty;

static int sChannel1FreqShadow;
static int sChannel1SweepTime;
static int sChannel1SweepTimer;
static int sChannel1SweepDir;
static int sChannel1SweepAmount;

static bool sChannel3IsMode64;
static int  sChannel3CurPlayBank;
static bool sChannel3IsEnabled;
static s8   sChannel3WaveData[2][32];

static int  sChannel4Div;
static bool sChannel4Is7Bit;
static int  sChannel4ShiftFreq;

static int sChannelEnableL;
static int sChannelEnableR;
static int sMasterVolumeL;
static int sMasterVolumeR;
static int sMixVolume;

static int sChannelPlaying;

static bool sMasterEnable;

static int gbDutyToDs(int gbDuty)
{
	if (gbDuty == 0)
		return 0; //12.5%
	else if (gbDuty == 1)
		return 1; //25%
	else if (gbDuty == 2)
		return 3; //50%
	else
		return 5; //75%
}

static void updateChannelDuty(int channel)
{
	switch (channel)
	{
		case 0:
			REG_SOUND[GB_CHANNEL_1_HW_L].CNT = (REG_SOUND[GB_CHANNEL_1_HW_L].CNT & ~REG_SOUNDXCNT_DUTY(7)) |
				REG_SOUNDXCNT_DUTY(gbDutyToDs(sChannel1Duty));
			REG_SOUND[GB_CHANNEL_1_HW_R].CNT = (REG_SOUND[GB_CHANNEL_1_HW_R].CNT & ~REG_SOUNDXCNT_DUTY(7)) |
				REG_SOUNDXCNT_DUTY(gbDutyToDs(sChannel1Duty));
			break;
		case 1:
			REG_SOUND[GB_CHANNEL_2_HW_L].CNT = (REG_SOUND[GB_CHANNEL_2_HW_L].CNT & ~REG_SOUNDXCNT_DUTY(7)) |
				REG_SOUNDXCNT_DUTY(gbDutyToDs(sChannel2Duty));
			REG_SOUND[GB_CHANNEL_2_HW_R].CNT = (REG_SOUND[GB_CHANNEL_2_HW_R].CNT & ~REG_SOUNDXCNT_DUTY(7)) |
				REG_SOUNDXCNT_DUTY(gbDutyToDs(sChannel2Duty));
			break;
	}
}

static void updateChannelFreq(int channel)
{
	switch (channel)
	{
		case 0:
			REG_SOUND[GB_CHANNEL_1_HW_L].TMR = (u16)(-(33513982 / 2) / (131072 / (2048 - sChannelFreq[0]) * 8));
			REG_SOUND[GB_CHANNEL_1_HW_R].TMR = (u16)(-(33513982 / 2) / (131072 / (2048 - sChannelFreq[0]) * 8));
			break;
		case 1:
			REG_SOUND[GB_CHANNEL_2_HW_L].TMR = (u16)(-(33513982 / 2) / (131072 / (2048 - sChannelFreq[1]) * 8));
			REG_SOUND[GB_CHANNEL_2_HW_R].TMR = (u16)(-(33513982 / 2) / (131072 / (2048 - sChannelFreq[1]) * 8));
			break;
		case 2:
		{
			int freq = 2097152 / (2048 - sChannelFreq[2]);
			REG_SOUND[GB_CHANNEL_3_HW_L_0].TMR = (u16)(-(33513982 / 2) / freq);
			REG_SOUND[GB_CHANNEL_3_HW_R_0].TMR = (u16)(-(33513982 / 2) / freq);
			REG_SOUND[GB_CHANNEL_3_HW_L_1].TMR = (u16)(-(33513982 / 2) / freq);
			REG_SOUND[GB_CHANNEL_3_HW_R_1].TMR = (u16)(-(33513982 / 2) / freq);
			break;
		}
		case 3:
			{
				int div = sChannel4Div * 8;
				if (div == 0)
					div = 4;
				div *= 2 << sChannel4ShiftFreq;
				int freq = 4194304 / div;
				freq *= 8;
				if (freq == 0)
				{
					REG_SOUND[GB_CHANNEL_4_HW_L].TMR = 0;
					REG_SOUND[GB_CHANNEL_4_HW_R].TMR = 0;
				}
				else
				{
					REG_SOUND[GB_CHANNEL_4_HW_L].TMR = (u16)(-(33513982 / 2) / freq);
					REG_SOUND[GB_CHANNEL_4_HW_R].TMR = (u16)(-(33513982 / 2) / freq);
				}
				break;
			}
	}
}

static void calcChannelVolume(int channel, int& volumeL, int& shiftL, int& volumeR, int& shiftR)
{
	if (sChannelEnableL & (1 << channel))
	{
		int sl = 2; //always at least a division by 4
		int vl = sChannelVolume[channel] * (sMasterVolumeL + 1);
		if (sMixVolume == 0)
			sl = 3; //division by 16
		else if (sMixVolume == 1)
			vl >>= 1; //sadly one bit is lost here
		volumeL = vl;
		shiftL = sl;
	}
	else
	{
		volumeL = 0;
		shiftL = 3;
	}

	if (sChannelEnableR & (1 << channel))
	{
		int sr = 2; //always at least a division by 4
		int vr = sChannelVolume[channel] * (sMasterVolumeR + 1);
		if (sMixVolume == 0)
			sr = 3; //division by 16
		else if (sMixVolume == 1)
			vr >>= 1; //sadly one bit is lost here
		volumeR = vr;
		shiftR = sr;
	}
	else
	{
		volumeR = 0;
		shiftR = 3;
	}
}

static void updateChannelVolume(int channel)
{
	int vl, sl, vr, sr;
	calcChannelVolume(channel, vl, sl, vr, sr);
	switch (channel)
	{
		case 0:
			REG_SOUND[GB_CHANNEL_1_HW_L].CNT = (REG_SOUND[GB_CHANNEL_1_HW_L].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
				REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vl) | REG_SOUNDXCNT_SHIFT(sl);
			REG_SOUND[GB_CHANNEL_1_HW_R].CNT = (REG_SOUND[GB_CHANNEL_1_HW_R].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
				REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vr) | REG_SOUNDXCNT_SHIFT(sr);
			break;
		case 1:
			REG_SOUND[GB_CHANNEL_2_HW_L].CNT = (REG_SOUND[GB_CHANNEL_2_HW_L].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
				REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vl) | REG_SOUNDXCNT_SHIFT(sl);
			REG_SOUND[GB_CHANNEL_2_HW_R].CNT = (REG_SOUND[GB_CHANNEL_2_HW_R].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
				REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vr) | REG_SOUNDXCNT_SHIFT(sr);
			break;
		case 2:
			if (sChannel3IsMode64 || sChannel3CurPlayBank == 0)
			{
				REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT = (REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vl) | REG_SOUNDXCNT_SHIFT(sl);				
				REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT = (REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vr) | REG_SOUNDXCNT_SHIFT(sr);
				REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT = (REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3)));
				REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT = (REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3)));
			}
			else
			{
				REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT = (REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3)));
				REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT = (REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3)));
				REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT = (REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vl) | REG_SOUNDXCNT_SHIFT(sl);
				REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT = (REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
					REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vr) | REG_SOUNDXCNT_SHIFT(sr);
			}
			break;
		case 3:
			REG_SOUND[GB_CHANNEL_4_HW_L].CNT = (REG_SOUND[GB_CHANNEL_4_HW_L].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
				REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vl) | REG_SOUNDXCNT_SHIFT(sl);
			REG_SOUND[GB_CHANNEL_4_HW_R].CNT = (REG_SOUND[GB_CHANNEL_4_HW_R].CNT & ~(REG_SOUNDXCNT_VOLUME(0x7F) |
				REG_SOUNDXCNT_SHIFT(3))) | REG_SOUNDXCNT_VOLUME(vr) | REG_SOUNDXCNT_SHIFT(sr);
			break;
	}
}

static void startChannel(int channel)
{
	int vl, sl, vr, sr, duty;
	calcChannelVolume(channel, vl, sl, vr, sr);
	switch (channel)
	{
		case 0:
			duty = gbDutyToDs(sChannel1Duty);
			REG_SOUND[GB_CHANNEL_1_HW_L].CNT = 0;
			REG_SOUND[GB_CHANNEL_1_HW_R].CNT = 0;
			REG_SOUND[GB_CHANNEL_1_HW_L].CNT =
				REG_SOUNDXCNT_E |
				REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PSG_NOISE) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
				REG_SOUNDXCNT_DUTY(duty) |
				REG_SOUNDXCNT_PAN(0) |
				REG_SOUNDXCNT_SHIFT(sl) |
				REG_SOUNDXCNT_VOLUME(vl);
			REG_SOUND[GB_CHANNEL_1_HW_R].CNT =
				REG_SOUNDXCNT_E |
				REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PSG_NOISE) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
				REG_SOUNDXCNT_DUTY(duty) |
				REG_SOUNDXCNT_PAN(127) |
				REG_SOUNDXCNT_SHIFT(sr) |
				REG_SOUNDXCNT_VOLUME(vr);
			break;
		case 1:
			duty = gbDutyToDs(sChannel2Duty);
			REG_SOUND[GB_CHANNEL_2_HW_L].CNT = 0;
			REG_SOUND[GB_CHANNEL_2_HW_R].CNT = 0;
			REG_SOUND[GB_CHANNEL_2_HW_L].CNT =
				REG_SOUNDXCNT_E |
				REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PSG_NOISE) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
				REG_SOUNDXCNT_DUTY(duty) |
				REG_SOUNDXCNT_PAN(0) |
				REG_SOUNDXCNT_SHIFT(sl) |
				REG_SOUNDXCNT_VOLUME(vl);
			REG_SOUND[GB_CHANNEL_2_HW_R].CNT =
				REG_SOUNDXCNT_E |
				REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PSG_NOISE) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
				REG_SOUNDXCNT_DUTY(duty) |
				REG_SOUNDXCNT_PAN(127) |
				REG_SOUNDXCNT_SHIFT(sr) |
				REG_SOUNDXCNT_VOLUME(vr);
			break;
		case 2:
			REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT = 0;
			REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT = 0;
			REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT = 0;
			REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT = 0;
			if(sChannel3IsMode64)
			{
				REG_SOUND[GB_CHANNEL_3_HW_L_0].SAD = (u32)&sChannel3WaveData[0][0];
				REG_SOUND[GB_CHANNEL_3_HW_R_0].SAD = (u32)&sChannel3WaveData[0][0];
				REG_SOUND[GB_CHANNEL_3_HW_L_0].LEN = 64 >> 2;
				REG_SOUND[GB_CHANNEL_3_HW_R_0].LEN = 64 >> 2;
				REG_SOUND[GB_CHANNEL_3_HW_L_0].PNT = 0;
				REG_SOUND[GB_CHANNEL_3_HW_R_0].PNT = 0;
				REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT =
					REG_SOUNDXCNT_E |
					REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
					REG_SOUNDXCNT_PAN(0) |
					REG_SOUNDXCNT_SHIFT(sl) |
					REG_SOUNDXCNT_VOLUME(vl);
				REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT =
					REG_SOUNDXCNT_E |
					REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
					REG_SOUNDXCNT_PAN(127) |
					REG_SOUNDXCNT_SHIFT(sr) |
					REG_SOUNDXCNT_VOLUME(vr);
			}
			else
			{
				REG_SOUND[GB_CHANNEL_3_HW_L_0].SAD = (u32)&sChannel3WaveData[0][0];
				REG_SOUND[GB_CHANNEL_3_HW_R_0].SAD = (u32)&sChannel3WaveData[0][0];
				REG_SOUND[GB_CHANNEL_3_HW_L_1].SAD = (u32)&sChannel3WaveData[1][0];
				REG_SOUND[GB_CHANNEL_3_HW_R_1].SAD = (u32)&sChannel3WaveData[1][0];
				REG_SOUND[GB_CHANNEL_3_HW_L_0].LEN = 32 >> 2;
				REG_SOUND[GB_CHANNEL_3_HW_R_0].LEN = 32 >> 2;
				REG_SOUND[GB_CHANNEL_3_HW_L_1].LEN = 32 >> 2;
				REG_SOUND[GB_CHANNEL_3_HW_R_1].LEN = 32 >> 2;
				REG_SOUND[GB_CHANNEL_3_HW_L_0].PNT = 0;
				REG_SOUND[GB_CHANNEL_3_HW_R_0].PNT = 0;
				REG_SOUND[GB_CHANNEL_3_HW_L_1].PNT = 0;
				REG_SOUND[GB_CHANNEL_3_HW_R_1].PNT = 0;
				REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT =
					REG_SOUNDXCNT_E |
					REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
					REG_SOUNDXCNT_PAN(0) |
					REG_SOUNDXCNT_SHIFT(sl) |
					REG_SOUNDXCNT_VOLUME(sChannel3CurPlayBank == 0 ? vl : 0);
				REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT =
					REG_SOUNDXCNT_E |
					REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
					REG_SOUNDXCNT_PAN(127) |
					REG_SOUNDXCNT_SHIFT(sr) |
					REG_SOUNDXCNT_VOLUME(sChannel3CurPlayBank == 0 ? vr : 0);
				REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT =
					REG_SOUNDXCNT_E |
					REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
					REG_SOUNDXCNT_PAN(0) |
					REG_SOUNDXCNT_SHIFT(sl) |
					REG_SOUNDXCNT_VOLUME(sChannel3CurPlayBank == 1 ? vl : 0);
				REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT =
					REG_SOUNDXCNT_E |
					REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PCM8) |
					REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
					REG_SOUNDXCNT_PAN(127) |
					REG_SOUNDXCNT_SHIFT(sr) |
					REG_SOUNDXCNT_VOLUME(sChannel3CurPlayBank == 1 ? vr : 0);
			}
			break;
		case 3:
			REG_SOUND[GB_CHANNEL_4_HW_L].CNT = 0;
			REG_SOUND[GB_CHANNEL_4_HW_R].CNT = 0;
			REG_SOUND[GB_CHANNEL_4_HW_L].CNT =
				REG_SOUNDXCNT_E |
				REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PSG_NOISE) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
				REG_SOUNDXCNT_PAN(0) |
				REG_SOUNDXCNT_SHIFT(sl) |
				REG_SOUNDXCNT_VOLUME(vl);
			REG_SOUND[GB_CHANNEL_4_HW_R].CNT =
				REG_SOUNDXCNT_E |
				REG_SOUNDXCNT_FORMAT(REG_SOUNDXCNT_FORMAT_PSG_NOISE) |
				REG_SOUNDXCNT_REPEAT(REG_SOUNDXCNT_REPEAT_LOOP) |
				REG_SOUNDXCNT_PAN(127) |
				REG_SOUNDXCNT_SHIFT(sr) |
				REG_SOUNDXCNT_VOLUME(vr);
			break;
	}
	sChannelPlaying |= 1 << channel;
	vram_cd->sound_emu_work.reg_gb_nr52 = sChannelPlaying | (sMasterEnable << 7);
}

static void stopChannel(int channel)
{
	switch (channel)
	{
		case 0:
			REG_SOUND[GB_CHANNEL_1_HW_L].CNT = 0;
			REG_SOUND[GB_CHANNEL_1_HW_R].CNT = 0;
			break;
		case 1:
			REG_SOUND[GB_CHANNEL_2_HW_L].CNT = 0;
			REG_SOUND[GB_CHANNEL_2_HW_R].CNT = 0;
			break;
		case 2:
			REG_SOUND[GB_CHANNEL_3_HW_L_0].CNT = 0;
			REG_SOUND[GB_CHANNEL_3_HW_R_0].CNT = 0;
			REG_SOUND[GB_CHANNEL_3_HW_L_1].CNT = 0;
			REG_SOUND[GB_CHANNEL_3_HW_R_1].CNT = 0;
			break;
		case 3:
			REG_SOUND[GB_CHANNEL_4_HW_L].CNT = 0;
			REG_SOUND[GB_CHANNEL_4_HW_R].CNT = 0;
			break;
	}
	sChannelPlaying &= ~(1 << channel);
	vram_cd->sound_emu_work.reg_gb_nr52 = sChannelPlaying | (sMasterEnable << 7);
}

/*
 * A length counter disables a channel when it decrements to zero. 
 * It contains an internal counter and enabled flag.
 * Writing a byte to NRx1 loads the counter with 64-data (256-data for wave channel). 
 * The counter can be reloaded at any time.
 *
 * A channel is said to be disabled when the internal enabled flag is clear. 
 * When a channel is disabled, its volume unit receives 0, otherwise its volume unit receives the output of the waveform generator.
 * Other units besides the length counter can enable/disable the channel as well.
 *
 * Each length counter is clocked at 256 Hz by the frame sequencer.
 * When clocked while enabled by NRx4 and the counter is not zero, it is decremented.
 * If it becomes zero, the channel is disabled.
 */
static void frameSeqUpdateLength()
{
	for (int i = 0; i < 4; i++)
		if ((sChannelPlaying & (1 << i)) && sChannelUseLen[i] && sChannelLengthCounter[i] > 0)
			if (--sChannelLengthCounter[i] == 0)
				stopChannel(i);
}

/*
 * A volume envelope has a volume counter and an internal timer clocked at 64 Hz by the frame sequencer.
 * When the timer generates a clock and the envelope period is not zero, a new volume is calculated by adding or subtracting (as set by NRx2) one from the current volume.
 * If this new volume within the 0 to 15 range, the volume is updated, otherwise it is left unchanged and no further automatic increments/decrements are made to the volume until the channel is triggered again.
 * 
 * When the waveform input is zero the envelope outputs zero, otherwise it outputs the current volume.
 * 
 * Writing to NRx2 causes obscure effects on the volume that differ on different Game Boy models (see obscure behavior).
 */
static void frameSeqUpdateVolume()
{
	for (int i = 0; i < 4; i++)
	{
		if (i == 2 || !(sChannelPlaying & (1 << i)))
			continue;
		if (sChannelEnvSweep[i] == 0)
			continue;
		if (++sChannelVolumeTimer[i] == sChannelEnvSweep[i])
		{
			int newVol = sChannelVolume[i] + sChannelEnvDir[i];
			if (newVol < 0)
				newVol = 0;
			if (newVol > 15)
				newVol = 15;
			sChannelVolume[i] = newVol;
			updateChannelVolume(i);
			sChannelVolumeTimer[i] = 0;
		}
	}
}

/*
 * The first square channel has a frequency sweep unit, controlled by NR10. 
 * This has a timer, internal enabled flag, and frequency shadow register. 
 * It can periodically adjust square 1's frequency up or down.
 * 
 * During a trigger event, several things occur:
 *		Square 1's frequency is copied to the shadow register.
 *		The sweep timer is reloaded.
 *		The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
 *		If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
 * Frequency calculation consists of taking the value in the frequency shadow register, shifting it right by sweep shift,
 * optionally negating the value, and summing this with the frequency shadow register to produce a new frequency.
 * What is done with this new frequency depends on the context.
 * 
 * The overflow check simply calculates the new frequency and if this is greater than 2047, square 1 is disabled.
 * 
 * The sweep timer is clocked at 128 Hz by the frame sequencer.
 * When it generates a clock and the sweep's internal enabled flag is set and the sweep period is not zero, a new frequency is calculated and the overflow check is performed.
 * If the new frequency is 2047 or less and the sweep shift is not zero, this new frequency is written back to the shadow frequency and square 1's frequency in NR13 and NR14,
 * then frequency calculation and overflow check are run AGAIN immediately using this new value, but this second new frequency is not written back.
 * 
 * Square 1's frequency can be modified via NR13 and NR14 while sweep is active, but the shadow frequency won't be affected so the next time the sweep updates the channel's frequency this modification will be lost.
 */
static void frameSeqUpdateSweep()
{
	if (!(sChannelPlaying & 1) || sChannel1SweepTime == 0)
		return;
	if (++sChannel1SweepTimer == sChannel1SweepTime)
	{
		int delta = (sChannel1FreqShadow >> sChannel1SweepAmount) * sChannel1SweepDir;
		if (sChannel1FreqShadow + delta >= 0 && sChannel1FreqShadow <= 2047)
		{
			sChannel1FreqShadow += delta;
			sChannelFreq[0] = sChannel1FreqShadow;
			updateChannelFreq(0);
		}
		else if (sChannel1FreqShadow + delta >= 2048)
			stopChannel(0);
		sChannel1SweepTimer = 0;
	}
}

//The frame sequencer generates low frequency clocks for the modulation units. It is clocked by a 512 Hz timer.
extern "C" void gbs_frameSeqTick()
{
	if (!sMasterEnable)
		return;
	// Step   Length Ctr  Vol Env     Sweep
	// ---------------------------------------
	// 0      Clock       -           -
	// 1      -           -           -
	// 2      Clock       -           Clock
	// 3      -           -           -
	// 4      Clock       -           -
	// 5      -           -           -
	// 6      Clock       -           Clock
	// 7      -           Clock       -
	// ---------------------------------------
	// Rate   256 Hz      64 Hz       128 Hz
	if ((sFrameSeqStep & 1) == 0)
		frameSeqUpdateLength();
	if ((sFrameSeqStep & 3) == 2)
		frameSeqUpdateSweep();
	if (sFrameSeqStep == 7)
		frameSeqUpdateVolume();

	sFrameSeqStep = (sFrameSeqStep + 1) & 7;
}

void gbs_init()
{
	sFrameSeqStep = 0;

	sChannelLength[0] = 0;
	sChannelLength[1] = 0;
	sChannelLength[2] = 0;
	sChannelLength[3] = 0;

	sChannelLengthCounter[0] = 0;
	sChannelLengthCounter[1] = 0;
	sChannelLengthCounter[2] = 0;
	sChannelLengthCounter[3] = 0;

	sChannelUseLen[0] = false;
	sChannelUseLen[1] = false;
	sChannelUseLen[2] = false;
	sChannelUseLen[3] = false;

	sChannelFreq[0] = 0;
	sChannelFreq[1] = 0;
	sChannelFreq[2] = 0;
	sChannelFreq[3] = 0;

	sChannelVolume[0] = 0;
	sChannelVolume[1] = 0;
	sChannelVolume[2] = 0;
	sChannelVolume[3] = 0;

	sChannelEnvDir[0] = 0;
	sChannelEnvDir[1] = 0;
	sChannelEnvDir[2] = 0;
	sChannelEnvDir[3] = 0;

	sChannelEnvSweep[0] = 0;
	sChannelEnvSweep[1] = 0;
	sChannelEnvSweep[2] = 0;
	sChannelEnvSweep[3] = 0;

	sChannelVolumeTimer[0] = 0;
	sChannelVolumeTimer[1] = 0;
	sChannelVolumeTimer[2] = 0;
	sChannelVolumeTimer[3] = 0;

	sChannel1Duty = 0;
	sChannel2Duty = 0;

	sChannel1SweepTime = 0;
	sChannel1SweepTimer = 0;
	sChannel1SweepDir = 0;
	sChannel1SweepAmount = 0;

	sChannel3IsMode64 = false;
	sChannel3CurPlayBank = 0;
	sChannel3IsEnabled = false;
	memset(sChannel3WaveData, 0, sizeof(sChannel3WaveData));

	sChannel4Div = 0;
	sChannel4Is7Bit = false;
	sChannel4ShiftFreq = 0;

	sChannelEnableL = 0;
	sChannelEnableR = 0;
	sMasterVolumeL = 0;
	sMasterVolumeR = 0;
	sMixVolume = 0;

	sChannelPlaying = 0;

	sMasterEnable = false;

	//setup timer 0 for the frame sequencer
	REG_TM[0].CNT_H = 0;
	REG_TM[0].CNT_L = TIMER_FREQ(512 * 1024);
	REG_TM[0].CNT_H = REG_TMXCNT_H_E | REG_TMXCNT_H_I | REG_TMXCNT_H_PS_1024;
	REG_IE |= (1 << 3);
}

//assumes an 8 bit write
void gbs_writeReg(u8 reg, u8 val)
{
	if (!sMasterEnable && reg != 0x84)
		return;
	switch (reg)
	{
		case 0x60: //NR10
			sChannel1SweepTime = (val >> 4) & 7;
			sChannel1SweepDir = (val & 8) ? -1 : 1;
			sChannel1SweepAmount = val & 7;
			break;
		case 0x62: //NR11
			sChannelLength[0] = val & 0x3F;
			sChannelLengthCounter[0] = 64 - sChannelLength[0];
			sChannel1Duty = val >> 6;
			updateChannelDuty(0);
			break;
		case 0x63: //NR12
			sChannelVolume[0] = val >> 4;
			sChannelEnvDir[0] = (val & 0x8) ? 1 : -1;
			sChannelEnvSweep[0] = val & 7;
			updateChannelVolume(0);
			break;
		case 0x64: //NR13
			sChannelFreq[0] = (sChannelFreq[0] & 0x700) | val;
			updateChannelFreq(0);
			break;
		case 0x65: //NR14
			sChannelFreq[0] = (sChannelFreq[0] & 0xFF) | ((val & 7) << 8);
			updateChannelFreq(0);
			sChannelUseLen[0] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[0] == 0)
					sChannelLengthCounter[0] = 64;
				sChannelVolume[0] = vram_cd->sound_emu_work.reg_gb_nr11_12 >> 12;
				sChannelVolumeTimer[0] = 0;
				sChannel1FreqShadow = sChannelFreq[0];
				sChannel1SweepTimer = 0;
				startChannel(0);
			}
			break;
		case 0x68: //NR21
			sChannelLength[1] = val & 0x3F;
			sChannelLengthCounter[1] = 64 - sChannelLength[1];
			sChannel2Duty = val >> 6;
			updateChannelDuty(1);
			break;
		case 0x69: //NR22
			sChannelVolume[1] = val >> 4;
			sChannelEnvDir[1] = (val & 0x8) ? 1 : -1;
			sChannelEnvSweep[1] = val & 7;
			updateChannelVolume(1);
			break;
		case 0x6C: //NR23
			sChannelFreq[1] = (sChannelFreq[1] & 0x700) | val;
			updateChannelFreq(1);
			break;
		case 0x6D: //NR24
			sChannelFreq[1] = (sChannelFreq[1] & 0xFF) | ((val & 7) << 8);
			updateChannelFreq(1);
			sChannelUseLen[1] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[1] == 0)
					sChannelLengthCounter[1] = 64;
				sChannelVolume[1] = vram_cd->sound_emu_work.reg_gb_nr21_22 >> 12;
				sChannelVolumeTimer[1] = 0;
				startChannel(1);
			}
			break;
		case 0x70: //NR30
			sChannel3IsMode64 = (val >> 5) & 1;
			sChannel3CurPlayBank = (val >> 6) & 1;
			sChannel3IsEnabled = (val >> 7) & 1;
			if (!sChannel3IsEnabled)
				stopChannel(2);
			else
				updateChannelVolume(2);
			break;
		case 0x72: //NR31
			sChannelLength[2] = val;
			sChannelLengthCounter[2] = 256 - sChannelLength[2];
			break;
		case 0x73: //NR32
			if (val & 0x80)
				sChannelVolume[2] = 12; //75%
			else
			{
				int vol = (val >> 5) & 3;
				if (vol == 0)
					sChannelVolume[2] = 0;
				else if (vol == 1)
					sChannelVolume[2] = 15;
				else if (vol == 2)
					sChannelVolume[2] = 8;
				else
					sChannelVolume[2] = 4;
			}
			updateChannelVolume(2);
			break;
		case 0x74: //NR33
			sChannelFreq[2] = (sChannelFreq[2] & 0x700) | val;
			updateChannelFreq(2);
			break;
		case 0x75: //NR34
			sChannelFreq[2] = (sChannelFreq[2] & 0xFF) | ((val & 7) << 8);
			updateChannelFreq(2);
			sChannelUseLen[2] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[2] == 0)
					sChannelLengthCounter[2] = 256;
				if(sChannel3IsEnabled)
					startChannel(2);
			}
			break;
		case 0x78: //NR41
			sChannelLength[3] = val & 0x3F;
			sChannelLengthCounter[3] = 64 - sChannelLength[3];
			break;
		case 0x79: //NR42
			sChannelVolume[3] = val >> 4;
			sChannelEnvDir[3] = (val & 0x8) ? 1 : -1;
			sChannelEnvSweep[3] = val & 7;
			updateChannelVolume(3);
			break;
		case 0x7C: //NR43
			sChannel4Div = val & 7;
			sChannel4Is7Bit = (val >> 3) & 1;
			sChannel4ShiftFreq = (val >> 4) & 0xF;
			updateChannelFreq(3);
			break;
		case 0x7D: //NR44
			sChannelUseLen[3] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[3] == 0)
					sChannelLengthCounter[3] = 64;
				sChannelVolume[3] = vram_cd->sound_emu_work.reg_gb_nr41_42 >> 12;
				sChannelVolumeTimer[3] = 0;
				startChannel(3);
			}
			break;
		case 0x80: //NR50
			sMasterVolumeL = (val >> 4) & 7;
			sMasterVolumeR = val & 7;
			updateChannelVolume(0);
			updateChannelVolume(1);
			updateChannelVolume(2);
			updateChannelVolume(3);
			break;
		case 0x81: //NR51
			sChannelEnableL = (val >> 4) & 0xF;
			sChannelEnableR = val & 0xF;
			updateChannelVolume(0);
			updateChannelVolume(1);
			updateChannelVolume(2);
			updateChannelVolume(3);
			break;

		case 0x84: //NR52
			if (!(val & 0x80))
			{
				sMasterEnable = false;
				//disable all sounds
				stopChannel(0);
				stopChannel(1);
				stopChannel(2);
				stopChannel(3);
			}
			else
				sMasterEnable = true;
			break;
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9A:
		case 0x9B:
		case 0x9C:
		case 0x9D:
		case 0x9E:
		case 0x9F:
			sChannel3WaveData[1 - sChannel3CurPlayBank][(reg & 0xF) << 1] = ((val & 0xF0) | (val >> 4)) - 128;
			sChannel3WaveData[1 - sChannel3CurPlayBank][((reg & 0xF) << 1) + 1] = (((val & 0xF) << 4) | (val & 0xF)) - 128;
			break;
	}
}

void gbs_setMixVolume(int mixVolume)
{
	sMixVolume = mixVolume;
	updateChannelVolume(0);
	updateChannelVolume(1);
	updateChannelVolume(2);
	updateChannelVolume(3);
}
