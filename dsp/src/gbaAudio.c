#include "teak/teak.h"
#include "gbaTimer.h"
#include "dmgAudio.h"
#include "gbaAudio.h"

static u32 sSyncClocksPerSample47[] =
{
    //GBAA_SYNC_MODE_DS_VSYNC
    0,
    //GBAA_SYNC_MODE_DS_TSYNC -> sample rate reduced by 0,9987944960594177
    23068672,
    //GBAA_SYNC_MODE_GBA_AJUSTED -> sample rate reduced by 0,9987944960594177
    23068672,
    //GBAA_SYNC_MODE_GBA_VSYNC -> this yields the perfect gba samplerate
    23096515,
    //GBAA_SYNC_MODE_GBA_TSYNC -> sample rate reduced by 0,9987944960594177
    23068672
};

static u32 sSyncClocksPerSample32[] =
{
    //GBAA_SYNC_MODE_DS_VSYNC
    0,
    //GBAA_SYNC_MODE_DS_TSYNC -> sample rate reduced by 0,9987944960594177
    33554432,
    //GBAA_SYNC_MODE_GBA_AJUSTED -> sample rate reduced by 0,9987944960594177
    33554432,
    //GBAA_SYNC_MODE_GBA_VSYNC -> this yields the perfect gba samplerate
    33594931,
    //GBAA_SYNC_MODE_GBA_TSYNC -> sample rate reduced by 0,9987944960594177
    33554432
};

gbaa_regs_t gGbaAudioRegs __attribute__((section(".data.sndregs")));

static gbaa_daudio_channel_t sDAudioChans[2];

static gbat_t sTimers[2];

static vu16 sDmaWasDone;
static vu16 sTransferBusy;

static vu16 sPaused;

static void initDChan(gbaa_daudio_channel_t* channel)
{
    for(short i = 0; i < 16; i++)
        channel->fifo[i] = 0;
    channel->readOffset = 0;
    channel->writeOffset = 0;
    channel->fifoCount = 0;    
    channel->dmaRequest = FALSE;
    channel->curPlaySamplesLo = 0;
    channel->curPlaySamplesHi = 0;
    channel->curPlaySampleCount = 0;
    channel->timerIdx = 0;
    channel->volume = 0;
    channel->enables = 0;
    channel->dmaControl = 0;
    channel->curDmaControl = 0;
    channel->dmaIrqMask = 0;
    channel->isDmaStarted = 0;
    channel->srcAddrLo = 0;
    channel->srcAddrHi = 0;
    channel->curSrcAddrLo = 0;
    channel->curSrcAddrHi = 0;
    channel->dstAddrLo = 0;
    channel->dstAddrHi = 0;
    channel->curDstAddrLo = 0;
    channel->curDstAddrHi = 0;
    channel->fifoAddrLo = 0;
    channel->curSample = 0;
    channel->isInitial = TRUE;
    channel->sampleCounterLo = 0;
    channel->sampleCounterHi = 0;
    channel->fetchedSampleCounterLo = 0;
    channel->fetchedSampleCounterHi = 0;
    channel->overrunCounter = 0;
}

void gbaa_init(void)
{
    gbat_initTimer(&sTimers[0]);
    gbat_initTimer(&sTimers[1]);

    initDChan(&sDAudioChans[0]);
    sDAudioChans[0].dmaIrqMask = 1 << 9;
    sDAudioChans[0].fifoAddrLo = 0xA0;
    initDChan(&sDAudioChans[1]);
    sDAudioChans[1].dmaIrqMask = 1 << 10;
    sDAudioChans[1].fifoAddrLo = 0xA4;

    dmga_init();

    sDmaWasDone = FALSE;
    sTransferBusy = FALSE;
    sPaused = TRUE;
}

void gbaa_start(void)
{
    // REG_TMR_CONTROL(0) = TMR_CONTROL_PAUSE;
    // //one interrupt per sample
    // REG_TMR_RELOAD_LO(0) = 2816 - 1;//(2816 * 1000) & 0xFFFF;//134055928 & 0xFFFF;
    // REG_TMR_RELOAD_HI(0) = 0;//(2816 * 1000) >> 16;//134055928 >> 16;
    cpu_disableIrqs();
    // REG_ICU_IRQ_ACK = ICU_IRQ_MASK_TMR0;
    // REG_ICU_IRQ_INT1 = ICU_IRQ_MASK_TMR0;
    // REG_ICU_IRQ_MODE |= ICU_IRQ_MASK_TMR0;
    // REG_ICU_IRQ_POLARITY &= ~ICU_IRQ_MASK_TMR0;
    // REG_ICU_IRQ_DISABLE &= ~ICU_IRQ_MASK_TMR0;

    REG_ICU_IRQ_ACK = ICU_IRQ_MASK_DMA;
    REG_ICU_IRQ_INT2 = ICU_IRQ_MASK_DMA;
    REG_ICU_IRQ_MODE |= ICU_IRQ_MASK_DMA;
    REG_ICU_IRQ_POLARITY &= ~ICU_IRQ_MASK_DMA;
    REG_ICU_IRQ_DISABLE &= ~ICU_IRQ_MASK_DMA;
    cpu_enableInt2();

    REG_BTDMP_TRANSMIT_UNK20(0) = 0xF | (2 << 8);// | (1 << 8); //the 1 is related to interrupt timing?
    REG_BTDMP_TRANSMIT_UNK22(0) = 0x1004;
    REG_BTDMP_TRANSMIT_UNK24(0) = 4;
    REG_BTDMP_TRANSMIT_UNK26(0) = 0x21;
    REG_BTDMP_TRANSMIT_UNK28(0) = 0;
    REG_BTDMP_TRANSMIT_UNK2A(0) = 0;
    btdmp_disableTransmit(0);
    btdmp_flushTransmitFifo(0);
    //for(short i = 0; i < 16; i++)
    //    REG_BTDMP_TRANSMIT_FIFO_DATA(0) = 0;

    REG_ICU_IRQ_ACK = ICU_IRQ_MASK_BTDMP0;
    REG_ICU_IRQ_INT1 = ICU_IRQ_MASK_BTDMP0;
    REG_ICU_IRQ_MODE &= ~ICU_IRQ_MASK_BTDMP0;
    REG_ICU_IRQ_POLARITY &= ~ICU_IRQ_MASK_BTDMP0;
    REG_ICU_IRQ_DISABLE &= ~ICU_IRQ_MASK_BTDMP0;

    btdmp_enableTransmit(0);

    //REG_TMR_CONTROL(0) = TMR_CONTROL_MODE(TMR_CONTROL_MODE_RELOAD) | TMR_CONTROL_BREAKPOINT | TMR_CONTROL_RESTART | TMR_CONTROL_UNFREEZE_COUNTER | TMR_CONTROL_AUTOCLEAR(TMR_CONTROL_AUTOCLEAR_2_CYCLES);

    cpu_enableInt1();
    cpu_enableIrqs();
}

void gbaa_stop(void)
{

}

static void setSemaphore(u32 val)
{
    //this seems to be very unsafe
    REG_APBP_PSEM |= val;
}

static void transferArm9ToDspDma5Async(u32 src, void* dst, u16 len)
{
    ahbm_resetChannel(1);

    REG_DMA_CHAN_SEL = 5; //set dma channel for config
    REG_DMA_CHAN_SRC_LO = src & 0xFFFF; //src address bit 15:0
    REG_DMA_CHAN_SRC_HI = src >> 16; //src address bit 31:16
    REG_DMA_CHAN_DST_LO = (u16)dst; //dst address bit 15:0
    REG_DMA_CHAN_DST_HI = 0; //dst address bit 31:16
    REG_DMA_CHAN_DIM0_LEN = len;
    REG_DMA_CHAN_DIM1_LEN = 1;
    REG_DMA_CHAN_DIM2_LEN = 1;
    REG_DMA_CHAN_DIM0_SRC_STEP = 4;
    REG_DMA_CHAN_DIM0_DST_STEP = 2;
    REG_DMA_CHAN_DIM1_SRC_STEP = 1;
    REG_DMA_CHAN_DIM1_DST_STEP = 1;
    REG_DMA_CHAN_DIM2_SRC_STEP = 1;
    REG_DMA_CHAN_DIM2_DST_STEP = 1;
    REG_DMA_CHAN_XFER_CONFIG =
        DMA_CHAN_XFER_CONFIG_SRC_REGION(DMA_MEMORY_REGION_ARM_AHBM) |
        DMA_CHAN_XFER_CONFIG_DST_REGION(DMA_MEMORY_REGION_DSP_DATA) |
        DMA_CHAN_XFER_CONFIG_RW_SIMULTANOUS | 
        DMA_CHAN_XFER_CONFIG_32BIT;
      
    REG_DMA_CHAN_UNK_81DC = 0x300; //idk

    u16 config = AHBM_CHAN_CONFIG1_SIZE(AHBM_CHAN_CONFIG1_SIZE_32BIT);
    //Important: Bursts with defined length may never cross 1024 byte boundaries!
    if((src & ~0x3FF) != ((src + (len << 1)) & ~0x3FF))
        config |= AHBM_CHAN_CONFIG1_BURST(AHBM_CHAN_CONFIG1_BURST_INCR);
    else
        config |= AHBM_CHAN_CONFIG1_BURST(AHBM_CHAN_CONFIG1_BURST_INCR4);
    ahbm_configChannel(1, config, 0x200, 1 << 5);

    REG_DMA_CHAN_CONTROL = 0x4000 | (1 << 2); //start
}

static void updateDChanDMA(gbaa_daudio_channel_t* channel)
{
    if(channel->isTransferring)// && sDmaWasDone)
    {
        if(!sDmaWasDone)
            return;

        channel->isTransferring = FALSE;
        sTransferBusy = FALSE;

        u16 newWOffset = channel->writeOffset + 8;
        if(newWOffset > 16)
        {
            newWOffset -= 16;
            for(u16 i = 0; i < newWOffset; i++)
                channel->fifo[i] = channel->fifoOverflow[i];
        }
        
        //I don't understand why it causes problems if I apply & 0xF here
        channel->writeOffset = newWOffset;
        channel->fifoCount += 16;

        u32 fSampCounter = channel->fetchedSampleCounterLo | (channel->fetchedSampleCounterHi << 16);
        fSampCounter += 16;
        channel->fetchedSampleCounterLo = fSampCounter & 0xFFFF;
        channel->fetchedSampleCounterHi = fSampCounter >> 16;
        //here we should send an irq to the arm9 if enabled in the dma channel
        if(channel->curDmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_IRQ)
            setSemaphore(channel->dmaIrqMask);
            //REG_APBP_PSEM = channel->dmaIrqMask; //TODO: check if writing a 0 does not destroy that bit

        //there are games that do not set the repeat bit, and manually enable
        //the channel again via an interrupt
        if(!(channel->curDmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_REPEAT))
        {
            channel->dmaControl &= ~GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE;
            channel->isDmaStarted = 0;
            //return;
        }
    }

    if(!(channel->dmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE))
    {
        channel->isDmaStarted = 0;
        return;
    }
    if(!channel->isDmaStarted)
    {
        if(channel->curDstAddrHi == 0x0400 && channel->curDstAddrLo == channel->fifoAddrLo)
        {
            u32 fSampCounter = channel->fetchedSampleCounterLo | (channel->fetchedSampleCounterHi << 16);

            if(channel->fifoCount <= -16)
            {
                //skip samples if we are behind by more than a complete dma transfer
                u16 skip = (-channel->fifoCount) & ~0xF;
                channel->fifoCount += skip;
                fSampCounter += skip;
                channel->writeOffset = (channel->writeOffset + (skip >> 1)) & 0xF;
            }

            u32 sampCounter = channel->sampleCounterLo | (channel->sampleCounterHi << 16);

            fSampCounter = (fSampCounter + 0xF) & ~0xF;
            sampCounter = (sampCounter + 0xF) & ~0xF;

            int diff = sampCounter - fSampCounter;
            if(diff <= -16)
            {
                channel->overrunCounter++;
                if(channel->overrunCounter >= 4)
                {
                    channel->fifoCount -= 4;
                    channel->writeOffset = (channel->writeOffset - 2) & 0xF;
                    channel->overrunCounter = 0;
                }
            }
        }

        //latch source address to address counter
        channel->curSrcAddrLo = channel->srcAddrLo;
        channel->curSrcAddrHi = channel->srcAddrHi;
        channel->curDstAddrLo = channel->dstAddrLo;
        channel->curDstAddrHi = channel->dstAddrHi;
        channel->curDmaControl = channel->dmaControl;
        if(channel->isTransferring)
            sTransferBusy = FALSE;
        channel->isTransferring = FALSE;
        channel->isDmaStarted = 1;

        channel->sampleCounterLo = 0;
        channel->sampleCounterHi = 0;
        channel->fetchedSampleCounterLo = 0;
        channel->fetchedSampleCounterHi = 0;
    }
    //do nothing if the dma is not in fifo mode
    if((channel->curDmaControl & GBAA_DAUDIO_CHANNEL_DMA_MODE_MASK) != GBAA_DAUDIO_CHANNEL_DMA_MODE_MASK)
        return;

    const gbat_t* timer = &sTimers[channel->timerIdx];
    if(!channel->dmaRequest && !(timer->isStarted && channel->fifoCount < 0))
        return;
    if(!sTransferBusy && !(REG_DMA_START & (1 << 5)))
    {
        channel->dmaRequest = FALSE;
        if(channel->curDstAddrHi != 0x0400 || channel->curDstAddrLo != channel->fifoAddrLo)
        {
            //this dma won't do anything to the fifo, but we will raise the irq if enabled
            if(channel->curDmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_IRQ)
                setSemaphore(channel->dmaIrqMask);

            if(!(channel->curDmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_REPEAT))
            {
                channel->dmaControl &= ~GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE;
                channel->isDmaStarted = 0;
            }
            return;
        }

        u32 addr = channel->curSrcAddrLo | ((u32)channel->curSrcAddrHi << 16);
        if(channel->fifoCount <= -16)
        {
            //skip samples if we are behind by more than a complete dma transfer
            u16 skip = (-channel->fifoCount) & ~0xF;
            channel->fifoCount += skip;
            addr += skip;
            u32 fSampCounter = channel->fetchedSampleCounterLo | (channel->fetchedSampleCounterHi << 16);
            fSampCounter += skip;
            channel->fetchedSampleCounterLo = fSampCounter & 0xFFFF;
            channel->fetchedSampleCounterHi = fSampCounter >> 16;            
            channel->writeOffset = (channel->writeOffset + (skip >> 1)) & 0xF;
        }
        channel->isTransferring = TRUE;
        sTransferBusy = TRUE;
        transferArm9ToDspDma5Async(addr, &channel->fifo[channel->writeOffset], 8);
        addr += 16;
        channel->curSrcAddrLo = addr & 0xFFFF;
        channel->curSrcAddrHi = addr >> 16;
    }
}

static void updateDChan(gbaa_daudio_channel_t* channel)
{
    updateDChanDMA(channel);
    const gbat_t* timer = &sTimers[channel->timerIdx];
    //more than 2 implies a samplerate higher than 64kHz
    if(timer->curNrOverflows > 10)//2)
        return;
    for(u16 i = 0; i < timer->curNrOverflows; i++)
    {
        u32 sampCounter = channel->sampleCounterLo | (channel->sampleCounterHi << 16);
        sampCounter++;
        channel->sampleCounterLo = sampCounter & 0xFFFF;
        channel->sampleCounterHi = sampCounter >> 16;
        if(channel->fifoCount <= 12 && !channel->isTransferring)
            channel->dmaRequest = TRUE;
        if(channel->curPlaySampleCount == 0)
        {
            if(channel->fifoCount >= 4)
            {
                channel->curPlaySamplesLo = channel->fifo[channel->readOffset];
                u16 newOffs = (channel->readOffset + 1) & 0xF;
                channel->curPlaySamplesHi = channel->fifo[newOffs];
                channel->isInitial = FALSE;
                channel->curPlaySampleCount = 4;
                channel->readOffset = (newOffs + 1) & 0xF;
                channel->fifoCount -= 4;
            }
            else
            {
                if (channel->isInitial)
                {                    
                    channel->isInitial = FALSE;
                    continue;
                }
                u16 samp = channel->curSample & 0xFF;
                samp |= samp << 8;
                channel->curPlaySamplesLo = samp;
                //channel->curPlaySamplesHi = samp;
                channel->curPlaySampleCount = 2;
                channel->readOffset = (channel->readOffset + 1) & 0xF;
                channel->fifoCount -= 2;
            }            
        }

        u32 curPlaySamples = channel->curPlaySamplesLo | (channel->curPlaySamplesHi << 16);
        channel->curSample = ((s16)((curPlaySamples & 0xFF) << 8)) >> 8;
        curPlaySamples >>= 8;
        channel->curPlaySamplesLo = curPlaySamples & 0xFFFF;
        channel->curPlaySamplesHi = curPlaySamples >> 16;
        channel->curPlaySampleCount--;
    }
    updateDChanDMA(channel);
}

s16 applyBias(int val)
{
    //todo: use real bias
    val += 0x200;
    if (val >= 0x400)
		val = 0x3FF;
	else if (val < 0)
		val = 0;
    return (val - 0x200) << 6;
}

void gbaa_updateDma(void)
{
    sDmaWasDone = (REG_DMA_START & (1 << 5)) == 0;
    if(sDmaWasDone)
        ahbm_resetChannel(1);
    updateDChanDMA(&sDAudioChans[0]);
    updateDChanDMA(&sDAudioChans[1]);
}

//because passing a stack address is not yet supported
static s16 sDmgSamp[2];

//called once per 32kHz sample
void gbaa_updateMixer(void)
{
    sDmaWasDone = (REG_DMA_START & (1 << 5)) == 0;
    if(sDmaWasDone)
        ahbm_resetChannel(1);

    int left = 0, right = 0;

    if(!sPaused)
    {
        gbat_updateTimer(&sTimers[0]);
        gbat_updateTimer(&sTimers[1]);
        
        //master enable
        if(gGbaAudioRegs.reg_gb_nr52 & 0x80)
        {
            updateDChan(&sDAudioChans[0]);
            updateDChan(&sDAudioChans[1]);

            dmga_sample(sDmgSamp);

            left = sDmgSamp[0];
            right = sDmgSamp[1];

            s16 sampA = sDAudioChans[0].curSample << 2;
            if(sDAudioChans[0].volume == 0)
                sampA = sampA >> 1;

            if(sDAudioChans[0].enables & 2)
                left += sampA;
            if(sDAudioChans[0].enables & 1)
                right += sampA;

            s16 sampB = sDAudioChans[1].curSample << 2;
            if(sDAudioChans[1].volume == 0)
                sampB = sampB >> 1;

            if(sDAudioChans[1].enables & 2)
                left += sampB;
            if(sDAudioChans[1].enables & 1)
                right += sampB;
        }
    }

    left = applyBias(left);
    right = applyBias(right);

    while(REG_BTDMP_TRANSMIT_FIFO_STAT(0) & BTDMP_TRANSMIT_FIFO_STAT_FULL);
    REG_BTDMP_TRANSMIT_FIFO_DATA(0) = left & 0xFFFF;
    while(REG_BTDMP_TRANSMIT_FIFO_STAT(0) & BTDMP_TRANSMIT_FIFO_STAT_FULL);
    REG_BTDMP_TRANSMIT_FIFO_DATA(0) = right & 0xFFFF;

    REG_ICU_IRQ_ACK = ICU_IRQ_MASK_BTDMP0;
}

static void resetFifo(gbaa_daudio_channel_t* channel)
{
    channel->fifoCount = 0;
    channel->readOffset = 0;
    channel->writeOffset = 0;
    if(channel->isTransferring)
        sTransferBusy = FALSE;
    channel->isTransferring = FALSE;
    channel->curPlaySampleCount = 0;
    channel->curPlaySamplesLo = 0;
    channel->curPlaySamplesHi = 0;
    channel->curSample = 0;
    channel->dmaRequest = FALSE;
    channel->isInitial = TRUE;
    channel->sampleCounterLo = 0;
    channel->sampleCounterHi = 0;
    channel->fetchedSampleCounterLo = 0;
    channel->fetchedSampleCounterHi = 0;
    channel->overrunCounter = 0;
}

static void writeFifo32(gbaa_daudio_channel_t* channel, u32 val, u32 mask)
{
    if(channel->fifoCount + 4 > 28)
    {
        channel->readOffset = 0;
        channel->writeOffset = 0;
        channel->fifoCount = 0;
    }
    else
    {
        u32 old = channel->fifo[channel->writeOffset] | (channel->fifo[channel->writeOffset + 1] << 16);
        old &= ~mask;
        old |= val & mask;
        channel->fifo[channel->writeOffset] = old & 0xFFFF;
        channel->fifo[channel->writeOffset + 1] = old >> 16;
        channel->writeOffset = (channel->writeOffset + 2) & 0xF;
        channel->fifoCount += 4;

        u32 fSampCounter = channel->fetchedSampleCounterLo | (channel->fetchedSampleCounterHi << 16);
        fSampCounter += 4;
        channel->fetchedSampleCounterLo = fSampCounter & 0xFFFF;
        channel->fetchedSampleCounterHi = fSampCounter >> 16;
    }
}

static void handleRegWrite(u16 regAddr, u16 length, u32 arg)
{
    if(regAddr == 0x100 && length == 2)
        sTimers[0].reload = arg & 0xFFFF;
    else if(regAddr == 0x102 && length == 2)
    {
        sTimers[0].control = arg & 0xFFFF;
        if(!(arg & GBAT_CONTROL_ENABLED))
            sTimers[0].isStarted = 0;
    }
    else if(regAddr == 0x100 && length == 4)
    {
        sTimers[0].reload = arg & 0xFFFF;
        sTimers[0].control = arg >> 16;
        if(!((arg >> 16) & GBAT_CONTROL_ENABLED))
            sTimers[0].isStarted = 0;
    }
    else if(regAddr == 0x104 && length == 2)
        sTimers[1].reload = arg & 0xFFFF;
    else if(regAddr == 0x106 && length == 2)
    {
        sTimers[1].control = arg & 0xFFFF;
        if(!(arg & GBAT_CONTROL_ENABLED))
            sTimers[1].isStarted = 0;
    }
    else if(regAddr == 0x104 && length == 4)
    {
        sTimers[1].reload = arg & 0xFFFF;
        sTimers[1].control = arg >> 16;
        if(!((arg >> 16) & GBAT_CONTROL_ENABLED))
            sTimers[1].isStarted = 0;
    }
    else if(regAddr == 0xBC && length == 4)
    {        
        sDAudioChans[0].srcAddrLo = arg & 0xFFFF;
        sDAudioChans[0].srcAddrHi = arg >> 16;
    }
    else if(regAddr == 0xC0 && length == 4)
    {        
        sDAudioChans[0].dstAddrLo = arg & 0xFFFF;
        sDAudioChans[0].dstAddrHi = arg >> 16;
    }
    else if(regAddr == 0xC4 && length == 4)
    {
        sDAudioChans[0].dmaControl = arg >> 16;
        if(!(sDAudioChans[0].dmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE))
            sDAudioChans[0].isDmaStarted = 0;
    }
    else if(regAddr == 0xC6 && length == 2)
    {
        sDAudioChans[0].dmaControl = arg & 0xFFFF;
        if(!(sDAudioChans[0].dmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE))
            sDAudioChans[0].isDmaStarted = 0;
    }
    else if(regAddr == 0xC8 && length == 4)
    {
        sDAudioChans[1].srcAddrLo = arg & 0xFFFF;
        sDAudioChans[1].srcAddrHi = arg >> 16;
    }
    else if(regAddr == 0xCC && length == 4)
    {
        sDAudioChans[1].dstAddrLo = arg & 0xFFFF;
        sDAudioChans[1].dstAddrHi = arg >> 16;
    }
    else if(regAddr == 0xD0 && length == 4)
    {
        sDAudioChans[1].dmaControl = arg >> 16;
        if(!(sDAudioChans[1].dmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE))
            sDAudioChans[1].isDmaStarted = 0;
    }
    else if(regAddr == 0xD2 && length == 2)
    {
        sDAudioChans[1].dmaControl = arg & 0xFFFF;
        if(!(sDAudioChans[1].dmaControl & GBAA_DAUDIO_CHANNEL_DMA_CONTROL_ENABLE))
            sDAudioChans[1].isDmaStarted = 0;
    }
    else if(regAddr >= 0xA0 && regAddr <= 0xA7)
    {
        u32 mask;
        if(length == 1)
            mask = 0xFF;
        else if(length == 2)
            mask = 0xFFFF;
        else
            mask = 0xFFFFFFFF;
        mask <<= (regAddr & 3) * 8;
        arg <<= (regAddr & 3) * 8;
        if(regAddr & 4)
            writeFifo32(&sDAudioChans[1], arg, mask);
        else
            writeFifo32(&sDAudioChans[0], arg, mask);
    }
    else if(regAddr >= 0x60 && regAddr < 0xB0)
    {
        for (u16 i = 0; i < length; i++)
        {
            if (regAddr == 0x82)
            {
                //SOUNDCNT_H
                gGbaAudioRegs.reg_gba_snd_mix = (gGbaAudioRegs.reg_gba_snd_mix & 0xFF00) | (arg & 0xFF);
                dmga_setMixVolume(arg & 3);

                sDAudioChans[0].volume = (arg >> 2) & 1;
                sDAudioChans[1].volume = (arg >> 3) & 1;
            }
            else if (regAddr == 0x83)
            {
                gGbaAudioRegs.reg_gba_snd_mix = (gGbaAudioRegs.reg_gba_snd_mix & 0xFF) | ((arg & 0x77) << 8);
                sDAudioChans[0].enables = arg & 3;
                sDAudioChans[0].timerIdx = (arg >> 2) & 1;
                if(arg & (1 << 3))
                    resetFifo(&sDAudioChans[0]);
                
                sDAudioChans[1].enables = (arg >> 4) & 3;
                sDAudioChans[1].timerIdx = (arg >> 6) & 1;
                if(arg & (1 << 7))
                    resetFifo(&sDAudioChans[1]);
            }
            else
                dmga_writeReg(regAddr & 0xFF, arg & 0xFF);
            regAddr++;
            arg >>= 8;
        }
    }
}

void gbaa_handleCommand(u32 cmd, u32 arg)
{
    u16 subCmd = cmd >> 24;
    if(subCmd == 0)
        handleRegWrite(cmd & 0xFFFF, (cmd >> 16) & 0xFF, arg);
    else if(subCmd == 1) //set pause
        sPaused = arg & 1;
}