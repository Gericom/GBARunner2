#include "teak/teak.h"
#include "gbaTimer.h"
#include "gbaAudio.h"
#include "dmg/dmgSweep.h"
#include "dmg/dmgEnvelope.h"
#include "dmgAudio.h"

static vu16 sMasterEnable;

//conveniently reuse a gba timer
static gbat_t sFrameSeqTimer;
static vu16 sFrameSeqStep;

static vu16 sChannelLength[4];
static vu16 sChannelLengthCounter[4];
static vu16 sChannelUseLen[4];

static vu16 sChannelFreq[4];
static vu16 sChannelVolume[4];
static vs16 sChannelEnvDir[4];
static vu16 sChannelEnvSweep[4];
static vu16 sChannelVolumeTimer[4];

static vu16 sChannel1Duty;
static vu16 sChannel2Duty;

static vu16 sChannel1FreqShadow;
static vu16 sChannel1SweepTime;
static vu16 sChannel1SweepTimer;
static vu16 sChannel1SweepDirNeg;
static vu16 sChannel1SweepAmount;

static vu16 sChannel3IsMode64;
static vu16 sChannel3CurPlayBank;
static vu16 sChannel3IsEnabled;
static vu16 sChannel3WaveData[16];

static vu16 sChannel3Ptr;

static vu16 sChannel4Div;
static vu16 sChannel4Is7Bit;
static vu16 sChannel4ShiftFreq;

static vu16 sChannelEnableL;
static vu16 sChannelEnableR;
static vu16 sMasterVolumeL;
static vu16 sMasterVolumeR;
static vu16 sMixVolume;

static vu16 sChannelPlaying;

static vu16 sChannel1PwmStep;
static vu16 sChannel2PwmStep;

static gbat_t sChannel1Timer;
static gbat_t sChannel2Timer;
static gbat_t sChannel3Timer;

static vu16 sChannel1Hi;
static vu16 sChannel2Hi;
static vu16 sChannel3Sample;
static vu16 sChannel4Hi;
static vu16 sChannel4Sample;

static vu16 sChannel4CounterLo;
static vu16 sChannel4CounterHi;

static vu16 sChannel4Lfsr;

static dmga_sweep_t sChannel1Sweep;

static dmga_envelope_t sChannel1Env;
static dmga_envelope_t sChannel2Env;
static dmga_envelope_t sChannel4Env;

void dmga_init(void)
{
	gGbaAudioRegs.reg_gb_nr10 = 0;
	gGbaAudioRegs.reg_gb_nr11_12 = 0;
	gGbaAudioRegs.reg_gb_nr13_14 = 0;
	gGbaAudioRegs.reg_gb_nr21_22 = 0;
	gGbaAudioRegs.reg_gb_nr23_24 = 0;
	gGbaAudioRegs.reg_gb_nr30 = 0;
	gGbaAudioRegs.reg_gb_nr31_32 = 0;
	gGbaAudioRegs.reg_gb_nr33_34 = 0;
	gGbaAudioRegs.reg_gb_nr41_42 = 0;
	gGbaAudioRegs.reg_gb_nr43_44 = 0;
	gGbaAudioRegs.reg_gb_nr50_51 = 0;
	gGbaAudioRegs.reg_gb_nr52 = 0;

    sFrameSeqStep = 0;

	sChannelLength[0] = 0;
	sChannelLength[1] = 0;
	sChannelLength[2] = 0;
	sChannelLength[3] = 0;

	sChannelLengthCounter[0] = 0;
	sChannelLengthCounter[1] = 0;
	sChannelLengthCounter[2] = 0;
	sChannelLengthCounter[3] = 0;

	sChannelUseLen[0] = FALSE;
	sChannelUseLen[1] = FALSE;
	sChannelUseLen[2] = FALSE;
	sChannelUseLen[3] = FALSE;

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
	sChannel1SweepDirNeg = FALSE;
	sChannel1SweepAmount = 0;

	sChannel3IsMode64 = FALSE;
	sChannel3CurPlayBank = 0;
	sChannel3IsEnabled = FALSE;
	//memset(sChannel3WaveData, 0, sizeof(sChannel3WaveData));

	sChannel4Div = 0;
	sChannel4Is7Bit = FALSE;
	sChannel4ShiftFreq = 0;

	sChannelEnableL = 0;
	sChannelEnableR = 0;
	sMasterVolumeL = 0;
	sMasterVolumeR = 0;
	sMixVolume = 0;

	sChannelPlaying = 0;

	sMasterEnable = FALSE;

	sChannel1PwmStep = 0;
	sChannel1Hi = 0;
	sChannel3Sample = 0;
	sChannel2Hi = 0;
	sChannel4Hi = 0;

	sChannel4CounterLo = 0;
	sChannel4CounterHi = 0;
	sChannel4Lfsr = 0;

	gbat_initTimer(&sChannel1Timer);
	sChannel1Timer.reload = -16 * 2048;
	sChannel1Timer.control = GBAT_CONTROL_ENABLED;

	gbat_initTimer(&sChannel2Timer);
	sChannel2Timer.reload = -16 * 2048;
	sChannel2Timer.control = GBAT_CONTROL_ENABLED;

	gbat_initTimer(&sChannel3Timer);
	sChannel3Timer.reload = -8 * 2048;
	sChannel3Timer.control = GBAT_CONTROL_ENABLED;

    //512 Hz frame seq timer
    gbat_initTimer(&sFrameSeqTimer);
    sFrameSeqTimer.reload = -32768;
    sFrameSeqTimer.control = GBAT_CONTROL_ENABLED;

	dmga_writeSweep(&sChannel1Sweep, 0);

	dmga_writeEnvelope(&sChannel1Env, 0);
	dmga_writeEnvelope(&sChannel2Env, 0);
	dmga_writeEnvelope(&sChannel4Env, 0);
}

static void updateChannelFreq(int channel)
{
	switch (channel)
	{
		case 0:
			sChannel1Timer.reload = -16 * (2048 - sChannelFreq[0]);
			break;
		case 1:
			sChannel2Timer.reload = -16 * (2048 - sChannelFreq[1]);
			break;
		case 2:
			sChannel3Timer.reload = -8 * (2048 - sChannelFreq[2]);
			break;
		case 3:
			break;
	}
}

static void startChannel(int channel)
{
	sChannelPlaying |= 1 << channel;
	gGbaAudioRegs.reg_gb_nr52 = sChannelPlaying | (sMasterEnable << 7);
}

void stopChannel(int channel)
{
	sChannelPlaying &= ~(1 << channel);
	gGbaAudioRegs.reg_gb_nr52 = sChannelPlaying | (sMasterEnable << 7);
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
static void frameSeqUpdateLength(void)
{
	for (s16 i = 0; i < 4; i++)
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
static void frameSeqUpdateVolume(void)
{
	// for (s16 i = 0; i < 4; i++)
	// {
	// 	if (i == 2 || !(sChannelPlaying & (1 << i)))
	// 		continue;
	// 	if (sChannelEnvSweep[i] == 0)
	// 		continue;
	// 	if (++sChannelVolumeTimer[i] == sChannelEnvSweep[i])
	// 	{
	// 		int newVol = sChannelVolume[i] + sChannelEnvDir[i];
	// 		if (newVol < 0)
	// 			newVol = 0;
	// 		if (newVol > 15)
	// 			newVol = 15;
	// 		sChannelVolume[i] = newVol;
	// 		//updateChannelVolume(i);
	// 		sChannelVolumeTimer[i] = 0;
	// 	}
	// }
	if(sChannelPlaying & 1)
		dmga_updateEnvelope(&sChannel1Env);
	if(sChannelPlaying & 2)
		dmga_updateEnvelope(&sChannel2Env);
	if(sChannelPlaying & 8)
		dmga_updateEnvelope(&sChannel4Env);
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
static void frameSeqUpdateSweep(void)
{
	// if (!(sChannelPlaying & 1) || sChannel1SweepTime == 0)
	// 	return;
	// if (++sChannel1SweepTimer == sChannel1SweepTime)
	// {
	// 	int delta = sChannel1FreqShadow >> sChannel1SweepAmount;// * sChannel1SweepDir;
	// 	if(sChannel1SweepDirNeg)
	// 		delta = -delta;
	// 	if (sChannel1FreqShadow + delta >= 0 && sChannel1FreqShadow <= 2047)
	// 	{
	// 		sChannel1FreqShadow += delta;
	// 		sChannelFreq[0] = sChannel1FreqShadow;
	// 		updateChannelFreq(0);
	// 	}
	// 	else if (sChannel1FreqShadow + delta >= 2048)
	// 		stopChannel(0);
	// 	sChannel1SweepTimer = 0;
	// }

	if(sChannel1Sweep.enabled)
	{
		if(--sChannel1Sweep.step == 0)
		{
			if(dmga_updateSweep(&sChannel1Sweep, 0, &sChannelFreq[0]))
				startChannel(0);
			else
				stopChannel(0);
			updateChannelFreq(0);
		}
	}
}

//The frame sequencer generates low frequency clocks for the modulation units.
//It is clocked by a 512 Hz timer.
static void frameSeqUpdate(void)
{
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

static void updateChannel1(void)
{
	if(!(sChannelPlaying & 1) || sChannel1Env.dead == 2)
		return;
	gbat_updateTimer(&sChannel1Timer);
	for(u16 i = 0; i < sChannel1Timer.curNrOverflows; i++)
	{
		switch(sChannel1Duty)
		{
			case 0:
				sChannel1Hi = sChannel1PwmStep == 7;
				break;
			case 1:
				sChannel1Hi = sChannel1PwmStep == 0 || sChannel1PwmStep == 7;
				break;
			case 2:
				sChannel1Hi = sChannel1PwmStep == 0 || sChannel1PwmStep >= 5;
				break;
			case 3:
				sChannel1Hi = sChannel1PwmStep != 0 && sChannel1PwmStep != 7;
				break;
		}
		sChannel1PwmStep = (sChannel1PwmStep + 1) & 7;
	}
}

static void updateChannel2(void)
{
 	if(!(sChannelPlaying & 2) || sChannel2Env.dead == 2)
		return;
	gbat_updateTimer(&sChannel2Timer);
	for(u16 i = 0; i < sChannel2Timer.curNrOverflows; i++)
	{
		switch(sChannel2Duty)
		{
			case 0:
				sChannel2Hi = sChannel2PwmStep == 7;
				break;
			case 1:
				sChannel2Hi = sChannel2PwmStep == 0 || sChannel2PwmStep == 7;
				break;
			case 2:
				sChannel2Hi = sChannel2PwmStep == 0 || sChannel2PwmStep >= 5;
				break;
			case 3:
				sChannel2Hi = sChannel2PwmStep != 0 && sChannel2PwmStep != 7;
				break;
		}
		sChannel2PwmStep = (sChannel2PwmStep + 1) & 7;
	}   
}

static void updateChannel3(void)
{
    if(!(sChannelPlaying & 4))
		return;
	gbat_updateTimer(&sChannel3Timer);
	u16 overflows = sChannel3Timer.curNrOverflows;
	if(!overflows)
		return;
	u16 ptr = sChannel3Ptr;
	if(overflows > 1)
	{
		ptr += overflows - 1;
		if(sChannel3IsMode64)
			ptr &= 0x3F;
		else
			ptr = (ptr & 0x1F) + (sChannel3CurPlayBank ? 32 : 0);
	}

	u16 samps = sChannel3WaveData[ptr >> 2];
	if(ptr & 2)
		samps >>= 8;
	if(!(ptr & 1))
		samps >>= 4;
	u16 samp = (u16)(samps & 0xF) * sChannelVolume[2];
	sChannel3Sample = samp >> 4;
	ptr++;
	if(sChannel3IsMode64)
		ptr &= 0x3F;
	else
		ptr = (ptr & 0x1F) + (sChannel3CurPlayBank ? 32 : 0);
	sChannel3Ptr = ptr;
}

//static u16 sDivisionTable[17] = 
//{
//	0, 1024, 512, 341, 256, 205, 171, 146, 128, 114, 102, 93, 85, 79, 73, 68, 64
//};

static void updateChannel4(void)
{
	if(!(sChannelPlaying & 8) || sChannel4Env.dead == 2)
		return;
    u32 cycles = sChannel4Div * 2;
	if(!cycles)
		cycles = 1;
	cycles <<= sChannel4ShiftFreq + 5;

	u32 counter = sChannel4CounterLo | ((u32)sChannel4CounterHi << 16);
	counter += 512;//352;	
	//u16 sampAcc = 0;
	//u16 sampCount = 0;
	while(counter >= cycles)
	{
		counter -= cycles;
		u16 lfsr = sChannel4Lfsr;
		u16 lsb = lfsr & 1;
		//sampAcc += lsb;
		//sampCount++;
		sChannel4Hi = lsb;
		lfsr >>= 1;
		u16 xor = sChannel4Is7Bit ? 0x60 : 0x6000;
		if(lsb)
			lfsr ^= xor;
		sChannel4Lfsr = lfsr;
	}
	// if(sampCount > 0)
	// {

	// }
	sChannel4CounterLo = counter & 0xFFFF;
	sChannel4CounterHi = counter >> 16;
}

void dmga_sample(s16* pSamp)
{
    if(!sMasterEnable)
    {
		pSamp[0] = 0;
		pSamp[1] = 0;
        return;
    }
    gbat_updateTimer(&sFrameSeqTimer);
    if(sFrameSeqTimer.curNrOverflows)
        frameSeqUpdate();
    updateChannel1();
    updateChannel2();
    updateChannel3();
    updateChannel4();
	s16 left = 0;
	s16 right = 0;

	s16 chan1Samp = sChannel1Hi ? sChannel1Env.curVolume : 0;
	if(sChannelEnableL & 1)
		left += chan1Samp;
	if(sChannelEnableR & 1)
		right += chan1Samp;

	s16 chan2Samp = sChannel2Hi ? sChannel2Env.curVolume : 0;
	if(sChannelEnableL & 2)
		left += chan2Samp;
	if(sChannelEnableR & 2)
		right += chan2Samp;

	s16 chan3Samp = sChannel3Sample;
	if(sChannelEnableL & 4)
		left += chan3Samp;
	if(sChannelEnableR & 4)
		right += chan3Samp;

	s16 chan4Samp = sChannel4Hi ? sChannel4Env.curVolume : 0;
	if(sChannelEnableL & 8)
		left += chan4Samp;
	if(sChannelEnableR & 8)
		right += chan4Samp;

	left <<= 3;
	right <<= 3;

	left *= 1 + sMasterVolumeL;
	right *= 1 + sMasterVolumeR;

	int shift = 4 - sMixVolume;
	left >>= shift;
	right >>= shift;

	pSamp[0] = left;
	pSamp[1] = right;
}

//assumes an 8 bit write
void dmga_writeReg(u16 reg, u16 val)
{
	if (!sMasterEnable && reg != 0x84)
		return;
	switch (reg)
	{
		case 0x60: //NR10
			gGbaAudioRegs.reg_gb_nr10 = val & ~0x80;
			if(!dmga_writeSweep(&sChannel1Sweep, val))
				stopChannel(0);
			//sChannel1SweepTime = (val >> 4) & 7;
			//sChannel1SweepDirNeg = (val & 8) ? TRUE : FALSE;
			//sChannel1SweepAmount = val & 7;
			break;
		case 0x62: //NR11
			gGbaAudioRegs.reg_gb_nr11_12 = (gGbaAudioRegs.reg_gb_nr11_12 & 0xFF00) | (val & ~0x3F);
			sChannelLength[0] = val & 0x3F;
			sChannelLengthCounter[0] = 64 - sChannelLength[0];
			sChannel1Duty = val >> 6;
			//updateChannelDuty(0);
			break;
		case 0x63: //NR12
			//sChannelVolume[0] = val >> 4;
			gGbaAudioRegs.reg_gb_nr11_12 = (gGbaAudioRegs.reg_gb_nr11_12 & 0xFF) | (val << 8);
			if(!dmga_writeEnvelope(&sChannel1Env, val))
				stopChannel(0);
			//sChannelEnvDir[0] = (val & 0x8) ? 1 : -1;
			//sChannelEnvSweep[0] = val & 7;
			//updateChannelVolume(0);
			break;
		case 0x64: //NR13
			sChannelFreq[0] = (sChannelFreq[0] & 0x700) | val;
			updateChannelFreq(0);
			break;
		case 0x65: //NR14
			gGbaAudioRegs.reg_gb_nr13_14 = (val & ~0xBF) << 8;
			sChannelFreq[0] = (sChannelFreq[0] & 0xFF) | ((val & 7) << 8);
			updateChannelFreq(0);
			sChannelUseLen[0] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[0] == 0)
					sChannelLengthCounter[0] = 64;
				//sChannelVolume[0] = gGbaAudioRegs.reg_gb_nr11_12 >> 12;
				//sChannelVolumeTimer[0] = 0;
				//sChannel1FreqShadow = sChannelFreq[0];
				//sChannel1SweepTimer = 0;
				sChannel1Sweep.realFreq = sChannelFreq[0];
				dmga_resetSweep(&sChannel1Sweep);
				if(dmga_resetEnvelope(&sChannel1Env))
					startChannel(0);
				else
					stopChannel(0);
			}
			break;
		case 0x68: //NR21
			gGbaAudioRegs.reg_gb_nr21_22 = (gGbaAudioRegs.reg_gb_nr21_22 & 0xFF00) | (val & ~0x3F);
			sChannelLength[1] = val & 0x3F;
			sChannelLengthCounter[1] = 64 - sChannelLength[1];
			sChannel2Duty = val >> 6;
			//updateChannelDuty(1);
			break;
		case 0x69: //NR22
			gGbaAudioRegs.reg_gb_nr21_22 = (gGbaAudioRegs.reg_gb_nr21_22 & 0xFF) | (val << 8);
			if(!dmga_writeEnvelope(&sChannel2Env, val))
				stopChannel(1);
			//sChannelVolume[1] = val >> 4;
			//sChannelEnvDir[1] = (val & 0x8) ? 1 : -1;
			//sChannelEnvSweep[1] = val & 7;
			//updateChannelVolume(1);
			break;
		case 0x6C: //NR23
			sChannelFreq[1] = (sChannelFreq[1] & 0x700) | val;
			updateChannelFreq(1);
			break;
		case 0x6D: //NR24
			gGbaAudioRegs.reg_gb_nr23_24 = (val & ~0xBF) << 8;
			sChannelFreq[1] = (sChannelFreq[1] & 0xFF) | ((val & 7) << 8);
			updateChannelFreq(1);
			sChannelUseLen[1] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[1] == 0)
					sChannelLengthCounter[1] = 64;
				//sChannelVolume[1] = gGbaAudioRegs.reg_gb_nr21_22 >> 12;
				//sChannelVolumeTimer[1] = 0;
				if(dmga_resetEnvelope(&sChannel2Env))
					startChannel(1);
				else
					stopChannel(1);
			}
			break;
		case 0x70: //NR30
			gGbaAudioRegs.reg_gb_nr30 = val & ~0x1F;
			sChannel3IsMode64 = (val >> 5) & 1;
			sChannel3CurPlayBank = (val >> 6) & 1;
			sChannel3IsEnabled = (val >> 7) & 1;
			if (!sChannel3IsEnabled)
				stopChannel(2);
			//else
				//updateChannelVolume(2);
			break;
		case 0x72: //NR31
			sChannelLength[2] = val;
			sChannelLengthCounter[2] = 256 - sChannelLength[2];
			break;
		case 0x73: //NR32
			gGbaAudioRegs.reg_gb_nr31_32 = (val & ~0x1F) << 8;
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
			//updateChannelVolume(2);
			break;
		case 0x74: //NR33
			sChannelFreq[2] = (sChannelFreq[2] & 0x700) | val;
			updateChannelFreq(2);
			break;
		case 0x75: //NR34
			gGbaAudioRegs.reg_gb_nr33_34 = (val & ~0xBF) << 8;
			sChannelFreq[2] = (sChannelFreq[2] & 0xFF) | ((val & 7) << 8);
			updateChannelFreq(2);
			sChannelUseLen[2] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[2] == 0)
					sChannelLengthCounter[2] = 256;
				if(sChannel3IsEnabled)
				{
					sChannel3Ptr = sChannel3CurPlayBank ? 32 : 0;
					startChannel(2);
				}
			}
			break;
		case 0x78: //NR41
			sChannelLength[3] = val & 0x3F;
			sChannelLengthCounter[3] = 64 - sChannelLength[3];
			break;
		case 0x79: //NR42
			gGbaAudioRegs.reg_gb_nr41_42 = val << 8;
			if(!dmga_writeEnvelope(&sChannel4Env, val))
				stopChannel(3);
			//sChannelVolume[3] = val >> 4;
			//sChannelEnvDir[3] = (val & 0x8) ? 1 : -1;
			//sChannelEnvSweep[3] = val & 7;
			//updateChannelVolume(3);
			break;
		case 0x7C: //NR43
			gGbaAudioRegs.reg_gb_nr43_44 = (gGbaAudioRegs.reg_gb_nr43_44 & 0xFF00) | val;
			sChannel4Div = val & 7;
			sChannel4Is7Bit = (val >> 3) & 1;
			sChannel4ShiftFreq = (val >> 4) & 0xF;
			updateChannelFreq(3);
			break;
		case 0x7D: //NR44
			gGbaAudioRegs.reg_gb_nr43_44 = (gGbaAudioRegs.reg_gb_nr43_44 & 0xFF) | ((val & ~0xBF) << 8);
			sChannelUseLen[3] = (val >> 6) & 1;
			if (val & 0x80)
			{
				if (sChannelLengthCounter[3] == 0)
					sChannelLengthCounter[3] = 64;
				//sChannelVolume[3] = gGbaAudioRegs.reg_gb_nr41_42 >> 12;
				//sChannelVolumeTimer[3] = 0;
				sChannel4CounterLo = 0;
				sChannel4CounterHi = 0;
				sChannel4Lfsr = sChannel4Is7Bit ? 0x7F : 0x7FFF;
				if(dmga_resetEnvelope(&sChannel4Env))
					startChannel(3);
				else
					stopChannel(3);
			}
			break;
		case 0x80: //NR50
			gGbaAudioRegs.reg_gb_nr50_51 = (gGbaAudioRegs.reg_gb_nr50_51 & 0xFF00) | val;
			sMasterVolumeL = (val >> 4) & 7;
			sMasterVolumeR = val & 7;
			// updateChannelVolume(0);
			// updateChannelVolume(1);
			// updateChannelVolume(2);
			// updateChannelVolume(3);
			break;
		case 0x81: //NR51
			gGbaAudioRegs.reg_gb_nr50_51 = (gGbaAudioRegs.reg_gb_nr50_51 & 0xFF) | (val << 8);
			sChannelEnableL = (val >> 4) & 0xF;
			sChannelEnableR = val & 0xF;
			// updateChannelVolume(0);
			// updateChannelVolume(1);
			// updateChannelVolume(2);
			// updateChannelVolume(3);
			break;

		case 0x84: //NR52
			gGbaAudioRegs.reg_gb_nr52 = (gGbaAudioRegs.reg_gb_nr52 & 0xF) | (val & 0x80);
			if (!(val & 0x80))
			{
				//disabling basically means reinitialization
				dmga_init();
				// sMasterEnable = FALSE;
				// //disable all sounds
				// stopChannel(0);
				// stopChannel(1);
				// stopChannel(2);
				// stopChannel(3);
			}
			else
				sMasterEnable = TRUE;
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
		{
			u16 bankOffset = (1 - sChannel3CurPlayBank) * 8;
			vu16* waveData = &sChannel3WaveData[bankOffset + ((reg & 0xF) >> 1)];
			if(reg & 1)
				*waveData = (*waveData & 0xFF) | (val << 8);
			else
				*waveData = (*waveData & 0xFF00) | val;
			//sChannel3WaveData[1 - sChannel3CurPlayBank][(reg & 0xF) << 1] = ((val & 0xF0) | (val >> 4)) - 128;
			//sChannel3WaveData[1 - sChannel3CurPlayBank][((reg & 0xF) << 1) + 1] = (((val & 0xF) << 4) | (val & 0xF)) - 128;
			break;
		}
	}
}

void dmga_setMixVolume(int mixVolume)
{
	sMixVolume = mixVolume;
	// updateChannelVolume(0);
	// updateChannelVolume(1);
	// updateChannelVolume(2);
	// updateChannelVolume(3);
}