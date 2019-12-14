#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "vramheap.h"
#include "sd_access.h"
#include "settings.h"
#include "gbaBoot.h"

extern u32 DISPCNT_copy;

void gbab_setupGfx()
{
    VRAM_D_CR = 0x80;
    VRAM_E_CR = 0x81;
	VRAM_F_CR = (DISPCNT_copy & 7) >= 3 ? 0x91 : 0x82;
	VRAM_G_CR = 0x8A;

    REG_BG3PA_SUB = 0x100;
    REG_BG3PB_SUB = 0;
    REG_BG3PC_SUB = 0;
    REG_BG3PD_SUB = 0x100;
    REG_BG3X_SUB =  -(8 * 256);
    REG_BG3Y_SUB = -(16 * 256);

    *(vu16*)0x04001040 = 0x8F8;
    *(vu16*)0x04001044 = 0x10B0;
    *(vu16*)0x04001048 = 0x18;
    *(vu16*)0x0400104A = 1;

    REG_BLDY_SUB = 0x10;
    REG_BLDCNT_SUB = 0x3FFF;

    REG_DISPCNT_SUB = 0x40013923;
    REG_BG0CNT_SUB = 0x1788;
    REG_BG3CNT_SUB = 0x4084 | (1 << 10);

    VRAM_C_CR = 0x84;
	VRAM_H_CR = 0x80; //H to lcdc
	VRAM_I_CR = 0x80; //I to lcdc

    for(int i = 0; i < 32 * 1024; i+=2)
        BG_GFX_SUB[(32 * 1024 + i) >> 1] = VRAM_I[i >> 1];

    VRAM_H_CR = gEmuSettingFrame ? 0x82 : 0x00;
    VRAM_I_CR = 0;

    u16 swap = gEmuSettingUseBottomScreen ^ gEmuSettingCenterMask ^ 1;
    *(vu16*)0x04000304 = ((*(vu16*)0x04000304) & ~0x800C) | (swap << 15);

    if(gEmuSettingCenterMask)
        REG_MASTER_BRIGHT = 0x801F;
    else
        REG_MASTER_BRIGHT_SUB = 0x801F;

    if(gEmuSettingCenterMask)
        REG_MASTER_BRIGHT_SUB = gEmuSettingGbaColors ? 0x8008 : 0;
    else
        REG_MASTER_BRIGHT = gEmuSettingGbaColors ? 0x8008 : 0;

    BG_PALETTE_SUB[0] = 0;
}