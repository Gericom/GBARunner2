#pragma once

#define GBAA_SYNC_MODE_DS_VSYNC     0
#define GBAA_SYNC_MODE_DS_TSYNC     1
#define GBAA_SYNC_MODE_GBA_AJUSTED  2
#define GBAA_SYNC_MODE_GBA_VSYNC    3
#define GBAA_SYNC_MODE_GBA_TSYNC    4

#define GBAA_DAUDIO_CHANNEL_DMA_CONTROL_REPEAT  (1 << 9)
#define GBAA_DAUDIO_CHANNEL_DMA_MODE_MASK       (3 << 12)
#define GBAA_DAUDIO_CHANNEL_DMA_CONTROL_IRQ     (1 << 14)
#define GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE  (1 << 15)

typedef struct
{
    vu16 fifo[16];
    vu16 fifoOverflow[8]; //to make dma transfers easier
    vu16 readOffset;
    vu16 writeOffset;
    vs16 fifoCount;
    vu16 dmaRequest;

    vu16 curPlaySamplesLo;
    vu16 curPlaySamplesHi;
    vu16 curPlaySampleCount;
    vu16 isInitial;

    vu16 timerIdx;
    u16 volume;
    u16 enables;

    vu16 dmaControl;
    vu16 curDmaControl; //latched version
    vu16 dmaIrqMask;
    vu16 isDmaStarted;
    vu16 srcAddrLo;
    vu16 srcAddrHi;
    vu16 addrLo;
    vu16 addrHi;
    vu16 isTransferring;

    vs16 curSample;

    vu16 sampleCounterLo;
    vu16 sampleCounterHi;

    vu16 fetchedSampleCounterLo;
    vu16 fetchedSampleCounterHi;

    vu16 overrunCounter;
} gbaa_daudio_channel_t __attribute__((aligned(4)));

typedef struct
{
    vu16 reg_gb_nr10;
	vu16 reg_gb_nr11_12;
	vu16 reg_gb_nr13_14;
    
    vu16 padding1;
	vu16 reg_gb_nr21_22;
	vu16 padding2;
    vu16 reg_gb_nr23_24;
    vu16 padding3;

	vu16 reg_gb_nr30;
	vu16 reg_gb_nr31_32;
	vu16 reg_gb_nr33_34;

    vu16 padding4;
	vu16 reg_gb_nr41_42;
    vu16 padding5;
	vu16 reg_gb_nr43_44;
    vu16 padding6;

	vu16 reg_gb_nr50_51;
	vu16 reg_gba_snd_mix;

	vu16 reg_gb_nr52;
    vu16 padding7;
} gbaa_regs_t;

extern gbaa_regs_t gGbaAudioRegs;

void gbaa_init(void);

void gbaa_start(void);

void gbaa_updateDma(void);
void gbaa_updateMixer(void);

void gbaa_handleCommand(u32 cmd, u32 arg);