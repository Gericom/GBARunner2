#include <nds/arm9/sprite.h>
#include <nds/arm9/background.h>
#include "vram.h"
#include "vramheap.h"
#include "string.h"
#include "sd_access.h"
#include "settings.h"
#include "cp15.h"
#include "save/Save.h"
#include "crc16.h"
#include "gamePatches.h"
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

    arm9_memcpy16(&BG_GFX_SUB[(32 * 1024) >> 1], VRAM_I, (32 * 1024) >> 1);

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

void gbab_setupCache()
{
    u32 regions = mpu_getICacheRegions();
    if(gEmuSettingWramICache)
        regions |= (1 << 0) | (1 << 7);
    else
        regions &= ~((1 << 0) | (1 << 7));

    if(gEmuSettingMainMemICache)
        regions |= 1 << 5;
    else
        regions &= ~(1 << 5);
    mpu_setICacheRegions(regions);
}

void gbab_loadFrame(u32 id)
{
	char framePath[] = "/_gba/frames/ABCD.bin";
	VRAM_I_CR = 0x80; //I to lcdc
	
	framePath[13] = id & 0xFF;
	framePath[14] = (id >> 8) & 0xFF;
	framePath[15] = (id >> 16) & 0xFF;
	framePath[16] = (id >> 24) & 0xFF;

	if (f_stat(framePath, NULL) == FR_OK)
		f_open(&vram_cd->fil, framePath, FA_OPEN_EXISTING | FA_READ);
	else if (f_stat("/_gba/frames/default.bin", NULL) == FR_OK)
		f_open(&vram_cd->fil, "/_gba/frames/default.bin", FA_OPEN_EXISTING | FA_READ);
	else
	{
        for(int i = 0; i < 128; i++)
		    ((u32*)0x06200000)[i] = 0;
        return;
    }

	UINT br;
	f_read(&vram_cd->fil, (void*)0x02020000, 512, &br);
	arm9_memcpy16((u16*)0x06200000, (u16*)0x02020000, 256);
	f_read(&vram_cd->fil, (void*)0x02020000, 2048, &br);
	arm9_memcpy16((u16*)0x068A3800, (u16*)0x02020000, 1024);
	f_read(&vram_cd->fil, (void*)0x02020000, 0x2A00, &br);
	arm9_memcpy16((u16*)0x068A0000, (u16*)0x02020000, 0x2A00 >> 1);
    f_close(&vram_cd->fil);
}

static RomLoadResult createLoadSave(const char* path, const save_type_t* saveType)
{
    if (saveType)
		vram_cd->save_work.saveSize = saveType->size;
	else
		vram_cd->save_work.saveSize = 64 * 1024;
	vram_cd->save_work.save_state = SAVE_WORK_STATE_CLEAN;
	if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
	{
		if (saveType && (saveType->type & SAVE_TYPE_TYPE_MASK) == SAVE_TYPE_FLASH)
		{
			for (int i = 0; i < vram_cd->save_work.saveSize >> 2; i++)
				((uint32_t*)MAIN_MEMORY_ADDRESS_SAVE_DATA)[i] = 0xFFFFFFFF;
		}
		else
		{
			for (int i = 0; i < vram_cd->save_work.saveSize >> 2; i++)
				((uint32_t*)MAIN_MEMORY_ADDRESS_SAVE_DATA)[i] = 0;
		}

#ifdef ISNITRODEBUG
		vram_cd->save_work.save_enabled = 0;
		return ROM_LOAD_RESULT_OK;
#else
		if (f_open(&vram_cd->fil, path, FA_CREATE_NEW | FA_WRITE) != FR_OK)
			return ROM_LOAD_RESULT_SAVE_CREATE_ERR;

		UINT bw;
		if (f_write(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_SAVE_DATA, vram_cd->save_work.saveSize, &bw) != FR_OK ||
			bw != vram_cd->save_work.saveSize)
			return ROM_LOAD_RESULT_SAVE_CREATE_ERR;
		f_close(&vram_cd->fil);
		if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
			return ROM_LOAD_RESULT_SAVE_CREATE_ERR;
#endif
	}

	if (saveType && (saveType->type & SAVE_TYPE_TYPE_MASK) == SAVE_TYPE_EEPROM && vram_cd->fil.obj.objsize == 512)
		vram_cd->save_work.saveSize = 512;

	if (vram_cd->fil.obj.objsize < vram_cd->save_work.saveSize)
		return ROM_LOAD_RESULT_SAVE_TOO_SMALL;

	uint32_t* cluster_table = &vram_cd->save_work.save_fat_table[0];
	uint32_t  cur_cluster = vram_cd->fil.obj.sclust;
	while (cur_cluster >= 2 && cur_cluster != 0xFFFFFFFF)
	{
		*cluster_table = f_clst2sect(&vram_cd->fatFs, cur_cluster);
		cluster_table++;
		cur_cluster = f_getFat(&vram_cd->fil, cur_cluster);
	}
	*cluster_table = 0;

	UINT br;
	if (f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_SAVE_DATA, vram_cd->save_work.saveSize, &br) != FR_OK ||
		br != vram_cd->save_work.saveSize)
		return ROM_LOAD_RESULT_SAVE_READ_ERR;
	f_close(&vram_cd->fil);

	vram_cd->save_work.fat_table_crc = crc16(0xFFFF, vram_cd->save_work.save_fat_table,
	                                         sizeof(vram_cd->save_work.save_fat_table));
#ifdef ISNITRODEBUG
	vram_cd->save_work.save_enabled = 0;
#else
	vram_cd->save_work.save_enabled = 1;
#endif

    return ROM_LOAD_RESULT_OK;
}

RomLoadResult gbab_loadRom(const char* path)
{
    if (f_open(&vram_cd->fil, path, FA_OPEN_EXISTING | FA_READ) != FR_OK)
		return ROM_LOAD_RESULT_ROM_READ_ERR;
	vram_cd->sd_info.gba_rom_size = vram_cd->fil.obj.objsize;
	uint32_t* cluster_table = &vram_cd->gba_rom_cluster_table[0];
	uint32_t  cur_cluster = vram_cd->fil.obj.sclust;
	while (cur_cluster >= 2 && cur_cluster != 0xFFFFFFFF)
	{
		*cluster_table = f_clst2sect(&vram_cd->fatFs, cur_cluster);
		cluster_table++;
		cur_cluster = f_getFat(&vram_cd->fil, cur_cluster);
	}
	UINT br;
	if (f_read(&vram_cd->fil, (void*)MAIN_MEMORY_ADDRESS_ROM_DATA, ROM_DATA_LENGTH, &br) != FR_OK)
		return ROM_LOAD_RESULT_ROM_READ_ERR;

	const save_type_t* saveType = save_findTag();
	if (saveType != NULL)
	{
		if (saveType->patchFunc != NULL)
			saveType->patchFunc(saveType);
	}

	f_close(&vram_cd->fil);

	gptc_patchRom();
    
    u32 gameCode = *(u32*)(MAIN_MEMORY_ADDRESS_ROM_DATA + 0xAC);
    gbab_loadFrame(gameCode);

    char nameBuf[256];
	for (int i = 0; i < 256; i++)
	{
		char c = path[i];
		nameBuf[i] = c;
		if (c == 0)
			break;
	}

	char* long_name_ptr = strrchr(nameBuf, '.');
	long_name_ptr[1] = 's';
	long_name_ptr[2] = 'a';
	long_name_ptr[3] = 'v';
	long_name_ptr[4] = '\0';

    return createLoadSave(nameBuf, saveType);
}