#pragma once

#define SCFG_RST_APPLY          0
#define SCFG_RST_RELEASE        1
#define REG_SCFG_RST            (*(vu16*)0x04004006)

#define SCFG_CLK_ARM9_134       (1 << 0)
#define SCFG_CLK_DSP_ENABLE     (1 << 1)
#define SCFG_CLK_CAM_IF_ENABLE  (1 << 2)
#define SCFG_CLK_NWRAM_ENABLED  (1 << 7)    //read only, set by arm7
#define SCFG_CLK_CAM_EXT_ENABLE (1 << 8)
#define REG_SCFG_CLK            (*(vu16*)0x04004004)


#define SCFG_EXT_REV_DMA            (1 << 0)
#define SCFG_EXT_REV_GE             (1 << 1)
#define SCFG_EXT_REV_RE             (1 << 2)
#define SCFG_EXT_REV_2D             (1 << 3)
#define SCFG_EXT_REV_DIV            (1 << 4)
#define SCFG_EXT_REV_CARD           (1 << 7)
#define SCFG_EXT_EXT_IRQ            (1 << 8)
#define SCFG_EXT_EXT_LCDC           (1 << 12)
#define SCFG_EXT_EXT_VRAM           (1 << 13)

#define SCFG_EXT_MAIN_MEM_SIZE_4M   0
#define SCFG_EXT_MAIN_MEM_SIZE_16M  2
#define SCFG_EXT_MAIN_MEM_SIZE_32M  3
#define SCFG_EXT_MAIN_MEM_SIZE(x)   ((x) << 14)

#define SCFG_EXT_ENABLE_NDMA        (1 << 16)
#define SCFG_EXT_ENABLE_CAM         (1 << 17)
#define SCFG_EXT_ENABLE_DSP         (1 << 18)
#define SCFG_EXT_CARD2_ENABLED      (1 << 24)    //read only, set by arm7
#define SCFG_EXT_NWRAM_ENABLED      (1 << 25)    //read only, set by arm7
#define SCFG_EXT_SCFG_MBK_ENABLED   (1 << 31)

#define REG_SCFG_EXT                (*(vu32*)0x04004008)