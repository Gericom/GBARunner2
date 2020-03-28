#include "teak.h"
#include "ahbm.h"
#include "dma.h"

void dma_init(void)
{
    //Apply settings to dma channel 0 that seem
    //to prevent lockups of the dma pipe or so
    u16 oldSel = REG_DMA_CHAN_SEL;
    REG_DMA_CHAN_SEL = 0;
    REG_DMA_START &= ~1;
    REG_DMA_CHAN_UNK_81DC &= ~0xF;
    REG_DMA_CHAN_UNK_81DC |= 0x10;
    REG_DMA_START |= 1;
    REG_DMA_CHAN_SEL = oldSel;
}

void dma_transferArm9ToDsp(u32 src, void* dst, u16 len)
{
    REG_DMA_CHAN_SEL = /*channel*/7; //set dma channel for config
    REG_DMA_CHAN_SRC_LO = src & 0xFFFF; //src address bit 15:0
    REG_DMA_CHAN_SRC_HI = src >> 16; //src address bit 31:16
    REG_DMA_CHAN_DST_LO = (u16)dst; //dst address bit 15:0
    REG_DMA_CHAN_DST_HI = 0; //dst address bit 31:16
    REG_DMA_CHAN_DIM0_LEN = len;
    REG_DMA_CHAN_DIM1_LEN = 1;
    REG_DMA_CHAN_DIM2_LEN = 1;
    REG_DMA_CHAN_DIM0_SRC_STEP = 2;
    REG_DMA_CHAN_DIM1_SRC_STEP = 1;
    REG_DMA_CHAN_DIM2_SRC_STEP = 1;
    REG_DMA_CHAN_DIM0_DST_STEP = 1;
    REG_DMA_CHAN_DIM1_DST_STEP = 1;
    REG_DMA_CHAN_DIM2_DST_STEP = 1;
    REG_DMA_CHAN_XFER_CONFIG = 0x207;
    REG_DMA_CHAN_UNK_81DC = 0x300; //idk

    ahbm_configChannel(2, 0x10, 0x200, 1 << 7/*channel*/);

    REG_DMA_CHAN_CONTROL = 0x4000; //start
    while((REG_DMA_DIM2_END & (1 << 7)) == 0);
    ahbm_resetChannel(2);
}

void dma_transferDspToArm9(/*u16 channel, */const void* src, u32 dst, u16 len)
{
    REG_DMA_CHAN_SEL = /*channel*/7; //set dma channel for config
    REG_DMA_CHAN_SRC_LO = (u16)src; //src address bit 15:0
    REG_DMA_CHAN_SRC_HI = 0x0000; //src address bit 31:16
    REG_DMA_CHAN_DST_LO = dst & 0xFFFF; //dst address bit 15:0
    REG_DMA_CHAN_DST_HI = dst >> 16; //dst address bit 31:16
    REG_DMA_CHAN_DIM0_LEN = len;
    REG_DMA_CHAN_DIM1_LEN = 1;
    REG_DMA_CHAN_DIM2_LEN = 1;
    REG_DMA_CHAN_DIM0_SRC_STEP = 1;
    REG_DMA_CHAN_DIM1_SRC_STEP = 1;
    REG_DMA_CHAN_DIM2_SRC_STEP = 1;
    REG_DMA_CHAN_DIM0_DST_STEP = 2;
    REG_DMA_CHAN_DIM1_DST_STEP = 2;
    REG_DMA_CHAN_DIM2_DST_STEP = 2;
    REG_DMA_CHAN_XFER_CONFIG = 0x270;
    REG_DMA_CHAN_UNK_81DC = 0x300; //idk

    ahbm_configChannel(2, 0x10, 0x300, 1 << 7/*channel*/);

    REG_DMA_CHAN_CONTROL = 0x4000; //start
    while((REG_DMA_DIM2_END & (1 << 7)) == 0);
    ahbm_resetChannel(2);
}