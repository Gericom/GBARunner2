#pragma once

#define REG_DMA_START              (*(vu16*)0x8184)
#define REG_DMA_PAUSE              (*(vu16*)0x8186)

#define REG_DMA_DIM0_END           (*(vu16*)0x8188)
#define REG_DMA_DIM1_END           (*(vu16*)0x818A)
#define REG_DMA_DIM2_END           (*(vu16*)0x818C)

#define REG_DMA_CHAN_SEL                (*(vu16*)0x81BE)
#define REG_DMA_CHAN_SRC_LO             (*(vu16*)0x81C0)
#define REG_DMA_CHAN_SRC_HI             (*(vu16*)0x81C2)
#define REG_DMA_CHAN_DST_LO             (*(vu16*)0x81C4)
#define REG_DMA_CHAN_DST_HI             (*(vu16*)0x81C6)
#define REG_DMA_CHAN_DIM0_LEN           (*(vu16*)0x81C8)
#define REG_DMA_CHAN_DIM1_LEN           (*(vu16*)0x81CA)
#define REG_DMA_CHAN_DIM2_LEN           (*(vu16*)0x81CC)
//signed!, watch out with backwards when performing burst access
#define REG_DMA_CHAN_DIM0_SRC_STEP      (*(vs16*)0x81CE)
#define REG_DMA_CHAN_DIM0_DST_STEP      (*(vs16*)0x81D0)
#define REG_DMA_CHAN_DIM1_SRC_STEP      (*(vs16*)0x81D2)
#define REG_DMA_CHAN_DIM1_DST_STEP      (*(vs16*)0x81D4)
#define REG_DMA_CHAN_DIM2_SRC_STEP      (*(vs16*)0x81D6)
#define REG_DMA_CHAN_DIM2_DST_STEP      (*(vs16*)0x81D8)

#define DMA_MEMORY_REGION_DSP_DATA          0
#define DMA_MEMORY_REGION_DSP_MMIO          1
#define DMA_MEMORY_REGION_DSP_CODE          5
#define DMA_MEMORY_REGION_ARM_AHBM          7

#define DMA_CHAN_XFER_CONFIG_SRC_REGION(x)  (x)
#define DMA_CHAN_XFER_CONFIG_DST_REGION(x)  ((x) << 4)

#define DMA_CHAN_XFER_CONFIG_RW_SIMULTANOUS (1 << 9)
#define DMA_CHAN_XFER_CONFIG_32BIT          (1 << 10)

#define REG_DMA_CHAN_XFER_CONFIG        (*(vu16*)0x81DA)
#define REG_DMA_CHAN_UNK_81DC           (*(vu16*)0x81DC)
#define REG_DMA_CHAN_CONTROL            (*(vu16*)0x81DE)

void dma_init(void);
void dma_transferArm9ToDsp(u32 src, void* dst, u16 len);
void dma_transferDspToArm9(/*u16 channel, */const void* src, u32 dst, u16 len);